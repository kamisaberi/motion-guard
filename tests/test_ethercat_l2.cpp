// File: /home/kami/motion-guard/tests/test_ethercat_l2.cpp

#include "motion_guard/pipeline/guard_engine.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[TEST] Non-IP L2 EtherCAT (0x88A4) Joint Surge & WKC Validation... ";
    motion_guard::pipeline::GuardEngine engine;

    std::vector<uint8_t> ecat(24, 0);
    const uint16_t hdr = static_cast<uint16_t>((1U << 12) | 22U);
    std::memcpy(&ecat[0], &hdr, 2);
    float accel = 4.2f, radius = 320.0f;
    std::memcpy(&ecat[12], &accel, 4);
    std::memcpy(&ecat[16], &radius, 4);
    uint16_t wkc = 1;
    std::memcpy(&ecat[20], &wkc, 2);
    assert(engine.process_ethercat_l2(ecat) == motion_guard::GuardStatus::PASS);

    accel = 48.0f;
    std::memcpy(&ecat[12], &accel, 4);
    assert(engine.process_ethercat_l2(ecat) == motion_guard::GuardStatus::REJECT_ETHERCAT_JOINT_SURGE);

    engine.reset_airgap();
    accel = 4.2f; wkc = 0;
    std::memcpy(&ecat[12], &accel, 4);
    std::memcpy(&ecat[20], &wkc, 2);
    assert(engine.process_ethercat_l2(ecat) == motion_guard::GuardStatus::REJECT_ETHERCAT_WKC_TAMPER);

    std::cout << "PASSED\n";
    return 0;
}
