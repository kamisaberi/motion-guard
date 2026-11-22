// File: /home/kami/motion-guard/tests/test_canfd_bus.cpp

#include "motion_guard/pipeline/guard_engine.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[TEST] SocketCAN / CAN-FD Arbitration ID & Actuator Override... ";
    motion_guard::pipeline::GuardEngine engine;

    std::vector<uint8_t> can(16, 0);
    uint32_t cid = 0x201;
    std::memcpy(&can[0], &cid, 4);
    can[4] = 8;
    float steer = 12.5f, torque = 85.0f;
    std::memcpy(&can[8], &steer, 4);
    std::memcpy(&can[12], &torque, 4);
    assert(engine.process_canfd(can) == motion_guard::GuardStatus::PASS);

    steer = 68.0f;
    std::memcpy(&can[8], &steer, 4);
    assert(engine.process_canfd(can) == motion_guard::GuardStatus::REJECT_CAN_ACTUATOR_OVERRIDE);

    engine.reset_airgap();
    cid = 0x7DF; steer = 0.0f;
    std::memcpy(&can[0], &cid, 4);
    std::memcpy(&can[8], &steer, 4);
    assert(engine.process_canfd(can) == motion_guard::GuardStatus::REJECT_CAN_UNAUTHORIZED_ID);

    std::cout << "PASSED\n";
    return 0;
}
