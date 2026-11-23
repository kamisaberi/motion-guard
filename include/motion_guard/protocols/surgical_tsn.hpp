// File: /home/kami/motion-guard/include/motion_guard/protocols/surgical_tsn.hpp

#pragma once

#include "motion_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace motion_guard::protocols {

struct SurgicalTsnFrame {
    uint32_t magic;        // 0x53555247 ("SURG")
    uint32_t cycle_seq;
    uint64_t tx_ptp_ns;    // Controller PTP hardware transmit timestamp
    float tip_x_mm;
    float tip_y_mm;
    float tip_z_mm;
    float force_n;
    float artery_x_mm;
    float artery_y_mm;
    float artery_z_mm;
};

struct SurgicalTsnPolicy {
    uint64_t nominal_cycle_ns{1'000'000ULL};  // 1 ms TSN 802.1Qbv gate cycle
    uint64_t max_cycle_jitter_ns{500'000ULL}; // +/- 500 us max deterministic jitter
    float min_vessel_clearance_mm{3.0f};      // 3D Keep-Out-Zone radius around major vessel
    float max_scalpel_force_n{8.0f};
};

class SurgicalTsnValidator {
public:
    static constexpr uint32_t MAGIC_SURG = 0x53555247U;

    [[nodiscard]] GuardStatus validate_frame(
        std::span<const uint8_t> payload,
        uint64_t rx_ptp_ns,
        const SurgicalTsnPolicy& policy = {}) noexcept;

    void reset() noexcept { last_rx_ns_ = 0; initialized_ = false; }

private:
    uint64_t last_rx_ns_{0};
    bool initialized_{false};
};

} // namespace motion_guard::protocols