// File: /home/kami/motion-guard/src/protocols/shm_motion.cpp

#include "motion_guard/protocols/shm_motion.hpp"
#include <cmath>
#include <cstring>

namespace motion_guard::protocols {

GuardStatus ShmMotionValidator::validate_slot(
    std::span<const uint8_t> payload,
    const ShmMotionPolicy& policy) noexcept
{
    if (payload.size() < sizeof(ShmMotionFrame)) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    ShmMotionFrame frame{};
    std::memcpy(&frame, payload.data(), sizeof(ShmMotionFrame));

    if (frame.magic != MAGIC_MOT1 ||
        !std::isfinite(frame.velocity_mps) ||
        !std::isfinite(frame.min_obstacle_m) ||
        !std::isfinite(frame.brake_temp_c)) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    if (frame.brake_temp_c > policy.max_brake_temp_c) {
        return GuardStatus::REJECT_SHM_BRAKE_OVERHEAT;
    }

    if (std::abs(frame.velocity_mps) > 0.2f && frame.min_obstacle_m < policy.min_safe_obstacle_m) {
        return GuardStatus::REJECT_SHM_OBSTACLE_COLLISION;
    }

    if (std::abs(frame.velocity_mps) > policy.max_velocity_mps) {
        return GuardStatus::REJECT_CBF_BOUNDARY_BREACH;
    }

    return GuardStatus::PASS;
}

} // namespace motion_guard::protocols
