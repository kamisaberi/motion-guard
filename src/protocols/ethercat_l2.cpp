// File: /home/kami/motion-guard/src/protocols/ethercat_l2.cpp

#include "motion_guard/protocols/ethercat_l2.hpp"
#include <cmath>
#include <cstring>

namespace motion_guard::protocols {

GuardStatus EthercatL2Validator::validate_pdu(
    std::span<const uint8_t> l2_payload,
    const EthercatPolicy& policy) noexcept
{
    if (l2_payload.size() < MIN_ECAT_PDU_LEN) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    // 2-byte EtherCAT Frame Header (LE): lower 11 bits = length, bits 12..15 = type (1 = EtherCAT)
    const uint16_t hdr = static_cast<uint16_t>(l2_payload[0]) | (static_cast<uint16_t>(l2_payload[1]) << 8);
    const uint16_t type = (hdr >> 12) & 0x0F;
    const uint16_t pdu_len = hdr & 0x07FF;

    if (type != 1 || pdu_len < 20 || (static_cast<size_t>(pdu_len) + 2U) > l2_payload.size()) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    // Datagram payload: 8 bytes [float joint_accel_mps2, float target_radius_mm] at offset 12..19
    float accel = 0.0f;
    float radius = 0.0f;
    std::memcpy(&accel, &l2_payload[12], 4);
    std::memcpy(&radius, &l2_payload[16], 4);

    if (!std::isfinite(accel) || !std::isfinite(radius)) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }
    if (std::abs(accel) > policy.max_joint_accel_mps2 || std::abs(radius) > policy.max_workspace_radius_mm) {
        return GuardStatus::REJECT_ETHERCAT_JOINT_SURGE;
    }

    // Working Counter (WKC) at end of datagram (bytes 20..21 LE)
    const uint16_t wkc = static_cast<uint16_t>(l2_payload[20]) | (static_cast<uint16_t>(l2_payload[21]) << 8);
    if (wkc < policy.min_expected_wkc) {
        return GuardStatus::REJECT_ETHERCAT_WKC_TAMPER;
    }

    return GuardStatus::PASS;
}

} // namespace motion_guard::protocols
