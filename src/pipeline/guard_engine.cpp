// File: /home/kami/motion-guard/src/pipeline/guard_engine.cpp

#include "motion_guard/pipeline/guard_engine.hpp"

namespace motion_guard::pipeline
{

    GuardStatus GuardEngine::process_ethercat_l2(
        std::span<const uint8_t> l2_payload,
        const protocols::EthercatPolicy &policy) noexcept
    {
        if (airgap_.is_tripped())
            return GuardStatus::MOTION_AIRGAP_TRIPPED;
        const GuardStatus st = protocols::EthercatL2Validator::validate_pdu(l2_payload, policy);
        if (st != GuardStatus::PASS)
            airgap_.trip();
        return st;
    }

    GuardStatus GuardEngine::process_canfd(
        std::span<const uint8_t> can_raw_frame,
        const protocols::CanFdPolicy &policy) noexcept
    {
        if (airgap_.is_tripped())
            return GuardStatus::MOTION_AIRGAP_TRIPPED;
        const GuardStatus st = protocols::CanFdBusValidator::validate_frame(can_raw_frame, policy);
        if (st != GuardStatus::PASS)
            airgap_.trip();
        return st;
    }

    GuardStatus GuardEngine::process_shm_motion(
        std::span<const uint8_t> shm_payload,
        const protocols::ShmMotionPolicy &policy) noexcept
    {
        if (airgap_.is_tripped())
            return GuardStatus::MOTION_AIRGAP_TRIPPED;
        const GuardStatus st = protocols::ShmMotionValidator::validate_slot(shm_payload, policy);
        if (st != GuardStatus::PASS)
            airgap_.trip();
        return st;
    }

    GuardStatus GuardEngine::process_surgical_tsn(
        std::span<const uint8_t> payload,
        uint64_t rx_ptp_ns,
        const protocols::SurgicalTsnPolicy &policy) noexcept
    {
        if (airgap_.is_tripped())
            return GuardStatus::MOTION_AIRGAP_TRIPPED;
        const GuardStatus st = surgical_tsn_.validate_frame(payload, rx_ptp_ns, policy);
        if (st != GuardStatus::PASS)
            airgap_.trip();
        return st;
    }

    GuardStatus GuardEngine::process_motion_cbf(
        const MotionState &state,
        const defense::MotionCbfLimits &limits) noexcept
    {
        if (airgap_.is_tripped())
            return GuardStatus::MOTION_AIRGAP_TRIPPED;
        const GuardStatus st = defense::MotionCbfGate::evaluate(state, limits);
        if (st != GuardStatus::PASS)
            airgap_.trip();
        return st;
    }

} // namespace motion_guard::pipeline