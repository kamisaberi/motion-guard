// File: /home/kami/motion-guard/tests/test_surgical_tsn.cpp

#include "motion_guard/pipeline/guard_engine.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[TEST] Surgical Robotics TSN 802.1Qbv Jitter & 3D Keep-Out-Zone CBF... ";
    motion_guard::pipeline::GuardEngine engine;

    motion_guard::protocols::SurgicalTsnFrame f{
        motion_guard::protocols::SurgicalTsnValidator::MAGIC_SURG, 1, 1'000'000ULL,
        10.0f, 10.0f, 5.0f, 2.5f, 0.0f, 0.0f, 0.0f};
    std::vector<uint8_t> buf(sizeof(f));
    std::memcpy(buf.data(), &f, sizeof(f));

    assert(engine.process_surgical_tsn(buf, 10'000'000ULL) == motion_guard::GuardStatus::PASS);
    // Nominal 1 ms TSN cycle (+50 us jitter < 500 us)
    f.cycle_seq = 2;
    std::memcpy(buf.data(), &f, sizeof(f));
    assert(engine.process_surgical_tsn(buf, 11'050'000ULL) == motion_guard::GuardStatus::PASS);

    // TSN Jitter Breach (+3 ms delay > 500 us)
    f.cycle_seq = 3;
    std::memcpy(buf.data(), &f, sizeof(f));
    assert(engine.process_surgical_tsn(buf, 15'050'000ULL) == motion_guard::GuardStatus::REJECT_TSN_JITTER_BREACH);

    // 3D Anatomical Keep-Out-Zone Breach (< 3.0 mm from artery)
    engine.reset_airgap();
    f.tip_x_mm = 1.0f; f.tip_y_mm = 1.0f; f.tip_z_mm = 0.0f; // dist = 1.41 mm < 3.0 mm
    std::memcpy(buf.data(), &f, sizeof(f));
    assert(engine.process_surgical_tsn(buf, 20'000'000ULL) == motion_guard::GuardStatus::REJECT_SURGICAL_KOZ_BREACH);

    std::cout << "PASSED\n";
    return 0;
}