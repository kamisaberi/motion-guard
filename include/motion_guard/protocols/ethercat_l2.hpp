// File: /home/kami/motion-guard/include/motion_guard/protocols/ethercat_l2.hpp

#pragma once

#include "motion_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace motion_guard::protocols {

struct EthercatPolicy {
    float max_joint_accel_mps2{15.0f};
    float max_workspace_radius_mm{850.0f};
    uint16_t min_expected_wkc{1};
};

class EthercatL2Validator {
public:
    static constexpr uint16_t ETHERTYPE_ETHERCAT = 0x88A4;
    static constexpr size_t MIN_ECAT_PDU_LEN = 22; // 2B header + 10B datagram hdr + 8B PDO + 2B WKC

    [[nodiscard]] static GuardStatus validate_pdu(
        std::span<const uint8_t> l2_payload,
        const EthercatPolicy& policy = {}) noexcept;
};

} // namespace motion_guard::protocols
