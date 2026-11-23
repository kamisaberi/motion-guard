// File: /home/kami/motion-guard/src/protocols/surgical_tsn.cpp

#include "motion_guard/protocols/surgical_tsn.hpp"
#include <cmath>
#include <cstring>

namespace motion_guard::protocols {

GuardStatus SurgicalTsnValidator::validate_frame(
    std::span<const uint8_t> payload,
    uint64_t rx_ptp_ns,
    const SurgicalTsnPolicy& policy) noexcept
{
    if (payload.size() < sizeof(SurgicalTsnFrame)) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    SurgicalTsnFrame f{};
    std::memcpy(&f, payload.data(), sizeof(SurgicalTsnFrame));

    if (f.magic != MAGIC_SURG ||
        !std::isfinite(f.tip_x_mm) || !std::isfinite(f.tip_y_mm) || !std::isfinite(f.tip_z_mm) ||
        !std::isfinite(f.artery_x_mm) || !std::isfinite(f.artery_y_mm) || !std::isfinite(f.artery_z_mm) ||
        !std::isfinite(f.force_n)) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    // 1. Hardware PTP / SO_TIMESTAMPING TSN 802.1Qbv Cycle Jitter Check
    if (initialized_ && rx_ptp_ns > 0 && last_rx_ns_ > 0) {
        const uint64_t dt = (rx_ptp_ns >= last_rx_ns_) ? (rx_ptp_ns - last_rx_ns_) : (last_rx_ns_ - rx_ptp_ns);
        const uint64_t jitter = (dt >= policy.nominal_cycle_ns)
                                    ? (dt - policy.nominal_cycle_ns)
                                    : (policy.nominal_cycle_ns - dt);
        if (jitter > policy.max_cycle_jitter_ns) {
            return GuardStatus::REJECT_TSN_JITTER_BREACH;
        }
    }
    last_rx_ns_ = rx_ptp_ns;
    initialized_ = true;

    // 2. 3D Anatomical Keep-Out-Zone CBF: h_koz(x) = ||p_tip - p_artery|| - r_safe >= 0
    const float dx = f.tip_x_mm - f.artery_x_mm;
    const float dy = f.tip_y_mm - f.artery_y_mm;
    const float dz = f.tip_z_mm - f.artery_z_mm;
    const float dist_mm = std::sqrt(dx * dx + dy * dy + dz * dz);
    const float h_koz = dist_mm - policy.min_vessel_clearance_mm;

    if (h_koz < 0.0f || f.force_n > policy.max_scalpel_force_n) {
        return GuardStatus::REJECT_SURGICAL_KOZ_BREACH;
    }

    return GuardStatus::PASS;
}

} // namespace motion_guard::protocols