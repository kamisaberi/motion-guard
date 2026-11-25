// File: /home/kami/motion-guard/tools/aryo_motion/main.cpp

#include "motion_guard/motion_guard.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <sentinel/fs/PathResolver.hpp>

static void print_help()
{
    std::cout << R"(================================================================================
ARYO MOTION: ROBOTICS, CAN-FD, ETHERCAT & NAGUMO CBF SAFETY (motion-guard)
================================================================================
Usage:
  aryo motion <action>

Actions:
  bench            Run 100,000-sample EtherCAT/CAN-FD benchmark (< 120 ns)
  verify-ethercat  Verify Non-IP L2 EtherCAT (0x88A4) joint surge clamp
  verify-can       Verify SocketCAN / CAN-FD steering & torque override block
  verify-shm       Verify POSIX ShmRing AMR obstacle & brake thermal guard
  verify-cbf       Verify Nagumo CBF forward invariance safety barrier
  airgap-status    Inspect real-time motion air-gap latch state
  cobot-up         Deploy robotics_can_ethercat_grid simulation mesh
  cobot-down       Teardown robotics_can_ethercat_grid simulation mesh
  cobot-status     Show running robotics_can_ethercat_grid containers
  --plugin-desc    Print single-line extension metadata for master CLI
)" << std::endl;
}

static std::string locate_scenario_engine()
{
    namespace fs = std::filesystem;
    if (const char *m_root = std::getenv("MATRIX_ROOT"); m_root && *m_root)
    {
        fs::path p = fs::path(m_root) / "tools" / "scenario_engine.py";
        if (fs::exists(p))
            return p.string();
    }
    if (const char *home = std::getenv("HOME"); home && *home)
    {
        fs::path p = fs::path(home) / "sentinel-matrix" / "tools" / "scenario_engine.py";
        if (fs::exists(p))
            return p.string();
    }
    if (fs::exists("/opt/sentinel-matrix/tools/scenario_engine.py"))
        return "/opt/sentinel-matrix/tools/scenario_engine.py";
    return "scenario_engine.py";
}

static std::string resolve_cartridge_scenario(const std::string &cartridge, const std::string &slug)
{
    namespace fs = std::filesystem;
    fs::path cat_dir = sentinel::fs::PathResolver::resolve_dir(sentinel::fs::ObjectClass::ScenarioCatalog) / slug;
    if (fs::exists(cat_dir))
        return cat_dir.string();

    fs::path pack_dir = sentinel::fs::PathResolver::resolve_pack_uri("pack://" + cartridge + "/scenarios/" + slug);
    if (fs::exists(pack_dir))
        return pack_dir.string();

    return slug;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_help();
        return 0;
    }

    const std::string action = argv[1];

    if (action == "--plugin-desc")
    {
        std::cout << "[ext. motion]           aryo motion (bench, verify-ethercat, verify-can, verify-shm, cobot-up) [Robotics & CAN-FD Guard]";
        return 0;
    }

    if (action == "-h" || action == "--help" || action == "help")
    {
        print_help();
        return 0;
    }

    if (action == "cobot-up")
    {
        const std::string cmd = "python3 \"" + locate_scenario_engine() + "\" load \"" +
                                resolve_cartridge_scenario("motion-guard", "robotics_can_ethercat_grid") + "\"";
        return std::system(cmd.c_str());
    }
    if (action == "cobot-down")
    {
        const std::string cmd = "python3 \"" + locate_scenario_engine() + "\" down --name robotics_can_ethercat_grid";
        return std::system(cmd.c_str());
    }
    if (action == "cobot-status")
    {
        const std::string cmd = "python3 \"" + locate_scenario_engine() + "\" status --name robotics_can_ethercat_grid";
        return std::system(cmd.c_str());
    }
    motion_guard::core::TscClock::calibrate();
    motion_guard::pipeline::GuardEngine engine;

    if (action == "bench")
    {
        constexpr size_t ITERATIONS = 100'000;
        std::vector<double> lat_ns;
        lat_ns.reserve(ITERATIONS);

        std::vector<uint8_t> ecat(24, 0);
        const uint16_t hdr = static_cast<uint16_t>((1U << 12) | 22U);
        std::memcpy(&ecat[0], &hdr, 2);
        float accel = 4.2f, radius = 320.0f;
        std::memcpy(&ecat[12], &accel, 4);
        std::memcpy(&ecat[16], &radius, 4);
        uint16_t wkc = 1;
        std::memcpy(&ecat[20], &wkc, 2);

        for (size_t i = 0; i < ITERATIONS; ++i)
        {
            const uint64_t t0 = motion_guard::core::TscClock::rdtsc();
            const auto st = engine.process_ethercat_l2(ecat);
            const uint64_t t1 = motion_guard::core::TscClock::rdtsc();
            if (st == motion_guard::GuardStatus::PASS)
            {
                lat_ns.push_back(motion_guard::core::TscClock::cycles_to_nanoseconds(t1 - t0));
            }
        }
        double total = 0.0;
        for (double v : lat_ns)
            total += v;
        std::cout << "[aryo-motion] Mean Latency (100k EtherCAT frames): " << (total / lat_ns.size()) << " ns\n";
        return 0;
    }

    if (action == "verify-ethercat")
    {
        std::vector<uint8_t> ecat(24, 0);
        const uint16_t hdr = static_cast<uint16_t>((1U << 12) | 22U);
        std::memcpy(&ecat[0], &hdr, 2);
        float accel = 48.0f, radius = 320.0f; // Surge > 15.0 m/s^2
        std::memcpy(&ecat[12], &accel, 4);
        std::memcpy(&ecat[16], &radius, 4);
        uint16_t wkc = 1;
        std::memcpy(&ecat[20], &wkc, 2);
        const auto st = engine.process_ethercat_l2(ecat);
        std::cout << "[aryo-motion] EtherCAT Joint Surge: " << motion_guard::status_to_string(st) << "\n";
        return (st == motion_guard::GuardStatus::REJECT_ETHERCAT_JOINT_SURGE) ? 0 : 1;
    }

    if (action == "verify-can")
    {
        std::vector<uint8_t> can(16, 0);
        uint32_t cid = 0x201;
        std::memcpy(&can[0], &cid, 4);
        can[4] = 8;
        float steer = 68.0f, torque = 120.0f; // Rogue steer > 35 deg
        std::memcpy(&can[8], &steer, 4);
        std::memcpy(&can[12], &torque, 4);
        const auto st = engine.process_canfd(can);
        std::cout << "[aryo-motion] CAN-FD Steer Override: " << motion_guard::status_to_string(st) << "\n";
        return (st == motion_guard::GuardStatus::REJECT_CAN_ACTUATOR_OVERRIDE) ? 0 : 1;
    }

    if (action == "verify-shm")
    {
        motion_guard::protocols::ShmMotionFrame f{motion_guard::protocols::ShmMotionValidator::MAGIC_MOT1, 1, 4.5f, 0.2f, 0.18f, 90.0f};
        std::vector<uint8_t> buf(sizeof(f));
        std::memcpy(buf.data(), &f, sizeof(f));
        const auto st = engine.process_shm_motion(buf);
        std::cout << "[aryo-motion] ShmRing Obstacle Stop: " << motion_guard::status_to_string(st) << "\n";
        return (st == motion_guard::GuardStatus::REJECT_SHM_OBSTACLE_COLLISION) ? 0 : 1;
    }

    if (action == "verify-cbf")
    {
        const motion_guard::MotionState ms{200.0, 2.3, 3.0, 50.0}; // dh/dt + 4*h = -3 + 4*(0.2) = -2.2 < 0!
        const auto st = engine.process_motion_cbf(ms);
        std::cout << "[aryo-motion] Nagumo CBF Violation: " << motion_guard::status_to_string(st) << "\n";
        return (st == motion_guard::GuardStatus::REJECT_CBF_BOUNDARY_BREACH) ? 0 : 1;
    }

    if (action == "airgap-status")
    {
        std::cout << "[aryo-motion] Motion Air-Gap Tripped: " << (engine.is_airgap_tripped() ? "YES" : "NO") << "\n";
        return 0;
    }

    std::cerr << "[-] Unknown action: " << action << "\n";
    return 1;
}
