// File: /home/kami/motion-guard/tests/test_motion_tier1_tier2_pipeline.cpp

#include "motion_guard/motion_guard.hpp"
#include "sentinel/sdk/abi.hpp"
#include <xinfer/xinfer.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

namespace fs = std::filesystem;

static std::string resolve_model(const std::string &model_name)
{
    const std::vector<fs::path> paths = {
        fs::path("/home/kami/motion-guard/scenarios/robotics_can_ethercat_grid/models") / model_name,
        fs::path("/home/kami/sentinel-matrix/shared/models") / model_name,
        fs::path("/etc/sentinel/models") / model_name};
    for (const auto &p : paths)
    {
        if (fs::exists(p)) return p.string();
    }
    return "";
}

static float run_tier2_onnx(xinfer::Engine *engine, const std::vector<uint8_t> &pkt, float tier1_score)
{
    if (!engine) return tier1_score;
    try
    {
        auto &in_t = engine->get_input_tensor("input");
        float *in_ptr = in_t.data<float>();
        const size_t n = in_t.element_count();
        for (size_t i = 0; i < n; ++i)
        {
            in_ptr[i] = (i < pkt.size()) ? (static_cast<float>(pkt[i]) / 255.0f) : tier1_score;
        }
        engine->infer();
        auto &out_t = engine->get_output_tensor("scores");
        const float *out_ptr = out_t.data<float>();
        const size_t m = out_t.element_count();
        if (m == n && n > 1)
        {
            double mse = 0.0;
            for (size_t i = 0; i < m; ++i)
            {
                double d = static_cast<double>(in_ptr[i]) - static_cast<double>(out_ptr[i]);
                mse += d * d;
            }
            return std::clamp(std::max(tier1_score, static_cast<float>(std::sqrt(mse / m))), 0.0f, 1.0f);
        }
        return std::clamp(std::max(tier1_score, std::abs(out_ptr[0])), 0.0f, 1.0f);
    }
    catch (...)
    {
        return tier1_score;
    }
}

int main()
{
    std::cout << "================================================================================\n";
    std::cout << "   MOTION-GUARD: TIER-1 (ETHERCAT/CAN/SHM/CBF) vs TIER-2 (OPENVINO ONNX) TEST   \n";
    std::cout << "================================================================================\n";

    motion_guard::core::TscClock::calibrate();
    motion_guard::pipeline::GuardEngine engine;

    std::unique_ptr<xinfer::Engine> vibe_eng;
    const std::string mpath = resolve_model("vibration_spectral_ae.onnx");
    if (!mpath.empty())
    {
        vibe_eng = std::make_unique<xinfer::Engine>(xinfer::Target::OpenVINO);
        vibe_eng->load_model(mpath);
        std::cout << "[+] Loaded ONNX Tier-2 Engine: " << mpath << "\n";
    }

    // Tier-1 Deterministic Drops
    std::vector<uint8_t> ecat_bad(24, 0);
    const uint16_t hdr = static_cast<uint16_t>((1U << 12) | 22U);
    std::memcpy(&ecat_bad[0], &hdr, 2);
    float accel = 45.0f, rad = 300.0f;
    std::memcpy(&ecat_bad[12], &accel, 4);
    std::memcpy(&ecat_bad[16], &rad, 4);
    uint16_t wkc = 1;
    std::memcpy(&ecat_bad[20], &wkc, 2);
    assert(engine.process_ethercat_l2(ecat_bad) == motion_guard::GuardStatus::REJECT_ETHERCAT_JOINT_SURGE);
    std::cout << "  [1.1] EtherCAT Joint Surge     -> TIER-1 KERNEL_DROP\n";

    engine.reset_airgap();
    std::vector<uint8_t> can_bad(16, 0);
    uint32_t cid = 0x201;
    std::memcpy(&can_bad[0], &cid, 4);
    can_bad[4] = 8;
    float steer = 68.0f, torque = 120.0f;
    std::memcpy(&can_bad[8], &steer, 4);
    std::memcpy(&can_bad[12], &torque, 4);
    assert(engine.process_canfd(can_bad) == motion_guard::GuardStatus::REJECT_CAN_ACTUATOR_OVERRIDE);
    std::cout << "  [1.2] CAN-FD Steer Override    -> TIER-1 KERNEL_DROP\n";

    engine.reset_airgap();
    motion_guard::protocols::ShmMotionFrame hit{motion_guard::protocols::ShmMotionValidator::MAGIC_MOT1, 1, 5.0f, 0.2f, 0.12f, 95.0f};
    std::vector<uint8_t> sbuf(sizeof(hit));
    std::memcpy(sbuf.data(), &hit, sizeof(hit));
    assert(engine.process_shm_motion(sbuf) == motion_guard::GuardStatus::REJECT_SHM_OBSTACLE_COLLISION);
    std::cout << "  [1.3] ShmRing Collision Stop   -> TIER-1 KERNEL_DROP\n";

    // Tier-2 ONNX Behavioral Evaluation
    float s_ok = run_tier2_onnx(vibe_eng.get(), std::vector<uint8_t>(32, 5), 0.05f);
    float s_bad = run_tier2_onnx(vibe_eng.get(), std::vector<uint8_t>(32, 240), 0.05f);
    assert(s_ok < 0.85f && s_bad >= 0.85f);
    std::cout << "  [2.1] Tier-2 Vibration AE      -> Normal: " << s_ok << " (PASS) | Anomaly: " << s_bad << " (DROP)\n";
    std::cout << ">>> RESULT: SUCCESS (ALL MOTION-GUARD TIER-1 & TIER-2 INVARIANTS VERIFIED) <<<\n";
    return 0;
}
