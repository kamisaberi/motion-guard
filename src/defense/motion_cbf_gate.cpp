// File: /home/kami/motion-guard/src/defense/motion_cbf_gate.cpp

#include "motion_guard/defense/motion_cbf_gate.hpp"
#include <cmath>

namespace motion_guard::defense {

GuardStatus MotionCbfGate::evaluate(
    const MotionState& state,
    const MotionCbfLimits& limits) noexcept
{
    if (!std::isfinite(state.position_mm) || !std::isfinite(state.velocity_mps) ||
        !std::isfinite(state.accel_mps2) || !std::isfinite(state.torque_nm)) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    if (std::abs(state.position_mm) > limits.max_position_mm ||
        std::abs(state.torque_nm) > limits.max_torque_nm) {
        return GuardStatus::REJECT_CBF_BOUNDARY_BREACH;
    }

    // Nagumo Control Barrier Function: h_v(x) = v_max - |v| >= 0
    const double h_v = limits.max_velocity_mps - std::abs(state.velocity_mps);
    const double sign_v = (state.velocity_mps >= 0.0) ? 1.0 : -1.0;
    const double h_v_dot = -sign_v * state.accel_mps2;

    if (h_v < 0.0 || (h_v_dot + limits.nagumo_alpha * h_v) < 0.0) {
        return GuardStatus::REJECT_CBF_BOUNDARY_BREACH;
    }

    return GuardStatus::PASS;
}

} // namespace motion_guard::defense
