// File: /home/kami/motion-guard/src/protocols/canfd_bus.cpp

#include "motion_guard/protocols/canfd_bus.hpp"
#include <cmath>
#include <cstring>

namespace motion_guard::protocols {

GuardStatus CanFdBusValidator::validate_frame(
    std::span<const uint8_t> can_raw_frame,
    const CanFdPolicy& policy) noexcept
{
    if (can_raw_frame.size() < MIN_CAN_FRAME_LEN) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    // Linux struct can_frame / canfd_frame layout:
    // bytes 0..3: can_id (host order), byte 4: dlc/len, bytes 8..15: data[0..7]
    uint32_t raw_id = 0;
    std::memcpy(&raw_id, &can_raw_frame[0], 4);
    const uint32_t can_id = raw_id & 0x1FFFFFFFU;
    const uint8_t dlc = can_raw_frame[4];

    if (can_id < policy.min_allowed_id || can_id > policy.max_allowed_id) {
        return GuardStatus::REJECT_CAN_UNAUTHORIZED_ID;
    }

    if (dlc >= 8 && can_raw_frame.size() >= 16) {
        float steer_deg = 0.0f;
        float torque_nm = 0.0f;
        std::memcpy(&steer_deg, &can_raw_frame[8], 4);
        std::memcpy(&torque_nm, &can_raw_frame[12], 4);

        if (!std::isfinite(steer_deg) || !std::isfinite(torque_nm)) {
            return GuardStatus::REJECT_MALFORMED_FRAME;
        }
        if (std::abs(steer_deg) > policy.max_steer_deg || std::abs(torque_nm) > policy.max_torque_nm) {
            return GuardStatus::REJECT_CAN_ACTUATOR_OVERRIDE;
        }
    }

    return GuardStatus::PASS;
}

} // namespace motion_guard::protocols
