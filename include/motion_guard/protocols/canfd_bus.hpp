// File: /home/kami/motion-guard/include/motion_guard/protocols/canfd_bus.hpp

#pragma once

#include "motion_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace motion_guard::protocols {

struct CanFdPolicy {
    uint32_t min_allowed_id{0x100};
    uint32_t max_allowed_id{0x5FF};
    float max_steer_deg{35.0f};
    float max_torque_nm{250.0f};
};

class CanFdBusValidator {
public:
    static constexpr size_t MIN_CAN_FRAME_LEN = 16; // struct can_frame (16 bytes)

    [[nodiscard]] static GuardStatus validate_frame(
        std::span<const uint8_t> can_raw_frame,
        const CanFdPolicy& policy = {}) noexcept;
};

} // namespace motion_guard::protocols
