// File: /home/kami/motion-guard/include/motion_guard/pipeline/guard_engine.hpp

#pragma once

#include "motion_guard/core/types.hpp"
#include "motion_guard/protocols/ethercat_l2.hpp"
#include "motion_guard/protocols/canfd_bus.hpp"
#include "motion_guard/protocols/shm_motion.hpp"
#include "motion_guard/defense/motion_cbf_gate.hpp"
#include "motion_guard/defense/motion_airgap.hpp"
#include "motion_guard/protocols/surgical_tsn.hpp"
#include <span>

namespace motion_guard::pipeline {

class GuardEngine {
public:
    [[nodiscard]] GuardStatus process_ethercat_l2(
        std::span<const uint8_t> l2_payload,
        const protocols::EthercatPolicy& policy = {}) noexcept;

    [[nodiscard]] GuardStatus process_canfd(
        std::span<const uint8_t> can_raw_frame,
        const protocols::CanFdPolicy& policy = {}) noexcept;

    [[nodiscard]] GuardStatus process_shm_motion(
        std::span<const uint8_t> shm_payload,
        const protocols::ShmMotionPolicy& policy = {}) noexcept;

    [[nodiscard]] GuardStatus process_motion_cbf(
        const MotionState& state,
        const defense::MotionCbfLimits& limits = {}) noexcept;

    [[nodiscard]] bool is_airgap_tripped() const noexcept { return airgap_.is_tripped(); }
    void reset_airgap() noexcept { airgap_.reset(); }

private:
    defense::MotionAirGap airgap_;
};

} // namespace motion_guard::pipeline
