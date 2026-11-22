// File: /home/kami/motion-guard/include/motion_guard/protocols/shm_motion.hpp

#pragma once

#include "motion_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace motion_guard::protocols {

struct ShmMotionFrame {
    uint32_t magic;          // 0x4D4F5431 ("MOT1")
    uint32_t node_id;
    float velocity_mps;
    float accel_mps2;
    float min_obstacle_m;
    float brake_temp_c;
};

struct ShmMotionPolicy {
    float max_velocity_mps{12.0f};
    float min_safe_obstacle_m{0.50f};
    float max_brake_temp_c{320.0f};
};

class ShmMotionValidator {
public:
    static constexpr uint32_t MAGIC_MOT1 = 0x4D4F5431U;

    [[nodiscard]] static GuardStatus validate_slot(
        std::span<const uint8_t> payload,
        const ShmMotionPolicy& policy = {}) noexcept;
};

} // namespace motion_guard::protocols
