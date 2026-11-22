// File: /home/kami/motion-guard/include/motion_guard/defense/motion_cbf_gate.hpp

#pragma once

#include "motion_guard/core/types.hpp"

namespace motion_guard::defense {

struct MotionCbfLimits {
    double max_position_mm{850.0};
    double max_velocity_mps{2.5};
    double max_torque_nm{120.0};
    double nagumo_alpha{4.0};
};

class MotionCbfGate {
public:
    /// Enforces Nagumo CBF forward invariance: h_v(x) = v_max - |v| >= 0 and dh/dt + alpha*h >= 0
    [[nodiscard]] static GuardStatus evaluate(
        const MotionState& state,
        const MotionCbfLimits& limits = {}) noexcept;
};

} // namespace motion_guard::defense
