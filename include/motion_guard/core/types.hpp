// File: /home/kami/motion-guard/include/motion_guard/core/types.hpp

#pragma once

#include <cstdint>
#include <string_view>

namespace motion_guard {

enum class GuardStatus : uint8_t {
    PASS = 0,
    REJECT_MALFORMED_FRAME,
    REJECT_ETHERCAT_JOINT_SURGE,
    REJECT_ETHERCAT_WKC_TAMPER,
    REJECT_CAN_UNAUTHORIZED_ID,
    REJECT_CAN_ACTUATOR_OVERRIDE,
    REJECT_SHM_OBSTACLE_COLLISION,
    REJECT_SHM_BRAKE_OVERHEAT,
    REJECT_CBF_BOUNDARY_BREACH,
    REJECT_TSN_JITTER_BREACH,
    REJECT_SURGICAL_KOZ_BREACH,
    MOTION_AIRGAP_TRIPPED
};

[[nodiscard]] constexpr std::string_view status_to_string(GuardStatus st) noexcept {
    switch (st) {
        case GuardStatus::PASS:                          return "PASS";
        case GuardStatus::REJECT_MALFORMED_FRAME:        return "REJECT_MALFORMED_FRAME";
        case GuardStatus::REJECT_ETHERCAT_JOINT_SURGE:   return "REJECT_ETHERCAT_JOINT_SURGE";
        case GuardStatus::REJECT_ETHERCAT_WKC_TAMPER:    return "REJECT_ETHERCAT_WKC_TAMPER";
        case GuardStatus::REJECT_CAN_UNAUTHORIZED_ID:    return "REJECT_CAN_UNAUTHORIZED_ID";
        case GuardStatus::REJECT_CAN_ACTUATOR_OVERRIDE:  return "REJECT_CAN_ACTUATOR_OVERRIDE";
        case GuardStatus::REJECT_SHM_OBSTACLE_COLLISION: return "REJECT_SHM_OBSTACLE_COLLISION";
        case GuardStatus::REJECT_SHM_BRAKE_OVERHEAT:     return "REJECT_SHM_BRAKE_OVERHEAT";
        case GuardStatus::REJECT_CBF_BOUNDARY_BREACH:    return "REJECT_CBF_BOUNDARY_BREACH";
        case GuardStatus::REJECT_TSN_JITTER_BREACH:      return "REJECT_TSN_JITTER_BREACH";
        case GuardStatus::REJECT_SURGICAL_KOZ_BREACH:    return "REJECT_SURGICAL_KOZ_BREACH";
        case GuardStatus::MOTION_AIRGAP_TRIPPED:         return "MOTION_AIRGAP_TRIPPED";
    }
    return "UNKNOWN";
}

struct MotionState {
    double position_mm{0.0};
    double velocity_mps{0.5};
    double accel_mps2{0.0};
    double torque_nm{12.0};
};

} // namespace motion_guard