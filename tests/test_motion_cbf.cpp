// File: /home/kami/motion-guard/tests/test_motion_cbf.cpp

#include "motion_guard/pipeline/guard_engine.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[TEST] ShmRing Motion Safety & Nagumo CBF Forward Invariance... ";
    motion_guard::pipeline::GuardEngine engine;

    motion_guard::protocols::ShmMotionFrame ok{motion_guard::protocols::ShmMotionValidator::MAGIC_MOT1, 1, 2.0f, 0.2f, 3.2f, 95.0f};
    std::vector<uint8_t> buf(sizeof(ok));
    std::memcpy(buf.data(), &ok, sizeof(ok));
    assert(engine.process_shm_motion(buf) == motion_guard::GuardStatus::PASS);

    motion_guard::protocols::ShmMotionFrame hit{motion_guard::protocols::ShmMotionValidator::MAGIC_MOT1, 1, 4.5f, 0.2f, 0.15f, 95.0f};
    std::memcpy(buf.data(), &hit, sizeof(hit));
    assert(engine.process_shm_motion(buf) == motion_guard::GuardStatus::REJECT_SHM_OBSTACLE_COLLISION);

    engine.reset_airgap();
    assert(engine.process_motion_cbf({120.0, 1.0, 0.5, 40.0}) == motion_guard::GuardStatus::PASS);
    assert(engine.process_motion_cbf({120.0, 2.3, 3.0, 40.0}) == motion_guard::GuardStatus::REJECT_CBF_BOUNDARY_BREACH);

    std::cout << "PASSED\n";
    return 0;
}
