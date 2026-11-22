// File: /home/kami/motion-guard/tools/motion_guard_ctl/main.cpp

#include "motion_guard/motion_guard.hpp"
#include <cstring>
#include <iostream>
#include <vector>

int main()
{
    motion_guard::core::TscClock::calibrate();
    motion_guard::pipeline::GuardEngine engine;

    std::vector<uint8_t> can(16, 0);
    uint32_t cid = 0x201;
    std::memcpy(&can[0], &cid, 4);
    can[4] = 8;
    float steer = 5.0f, torque = 45.0f;
    std::memcpy(&can[8], &steer, 4);
    std::memcpy(&can[12], &torque, 4);

    const auto st = engine.process_canfd(can);
    std::cout << "[motion-guard-ctl] TSC Clock : " << motion_guard::core::TscClock::ghz() << " GHz\n";
    std::cout << "[motion-guard-ctl] Self-Test : " << motion_guard::status_to_string(st) << "\n";
    return (st == motion_guard::GuardStatus::PASS) ? 0 : 1;
}
