// File: /home/kami/motion-guard/tools/sentinel_plugin/motion_guard_plugin.cpp

#include "sentinel/sdk/abi.hpp"
#include "motion_guard/motion_guard.hpp"
#include <algorithm>
#include <cstring>
#include <memory>
#include <span>
#include <string>

static const SentinelHostInterface *g_host = nullptr;
static std::unique_ptr<motion_guard::pipeline::GuardEngine> g_guard_engine;

static int plugin_init(const SentinelHostInterface *host_iface)
{
    g_host = host_iface;
    g_guard_engine = std::make_unique<motion_guard::pipeline::GuardEngine>();
    motion_guard::core::TscClock::calibrate();

    if (g_host && g_host->log_message)
    {
        g_host->log_message(1, "MotionGuard",
                            "Robotics EtherCAT 0x88A4, CAN-FD, ShmRing & Surgical TSN Plugin Initialized (< 120 ns SLA).");
    }
    return 0;
}

static void plugin_shutdown()
{
    g_guard_engine.reset();
    g_host = nullptr;
}

static SentinelDissectorResult plugin_dissect(const SentinelRawPacket *packet)
{
    SentinelDissectorResult res{};
    res.verdict = SENTINEL_VERDICT_PASS;
    res.threat_score = 0;
    res.rule_id = 0;
    res.semantic_tags = 0;
    res.threat_name[0] = '\0';

    if (!packet || !packet->data || packet->length < 16 || !g_guard_engine)
    {
        return res;
    }

    motion_guard::GuardStatus st = motion_guard::GuardStatus::PASS;
    uint32_t matched_rule_id = 0;

    // 1. Bus B: SocketCAN / AF_UNIX CAN-FD (reserved == 1)
    if (packet->reserved == 1)
    {
        st = g_guard_engine->process_canfd(std::span<const uint8_t>(packet->data, packet->length));
        matched_rule_id = 0x0CA4;
    }
    // 2. Bus C: POSIX /dev/shm Ring (reserved == 2) — dispatches "SURG" or "MOT1" cleanly
    else if (packet->reserved == 2)
    {
        uint32_t magic = 0;
        std::memcpy(&magic, packet->data, 4);
        const std::span<const uint8_t> payload(packet->data, packet->length);

        if (magic == motion_guard::protocols::SurgicalTsnValidator::MAGIC_SURG)
        {
            st = g_guard_engine->process_surgical_tsn(payload, packet->timestamp_ns);
            matched_rule_id = 0x5355;
        }
        else if (magic == motion_guard::protocols::ShmMotionValidator::MAGIC_MOT1)
        {
            st = g_guard_engine->process_shm_motion(payload);
            matched_rule_id = 0x0530;
        }
    }
    // 3. Bus A (Non-IP L2): EtherCAT 0x88A4
    else if (packet->network_proto == motion_guard::protocols::EthercatL2Validator::ETHERTYPE_ETHERCAT)
    {
        const uint8_t *pdu = packet->data;
        size_t pdu_len = packet->length;

        if (packet->length >= 36)
        {
            const uint16_t et12 = (static_cast<uint16_t>(packet->data[12]) << 8) | packet->data[13];
            if (et12 == 0x88A4)
            {
                pdu = packet->data + 14;
                pdu_len = packet->length - 14;
            }
            else if (et12 == 0x8100 && packet->length >= 40)
            {
                pdu = packet->data + 18;
                pdu_len = packet->length - 18;
            }
        }

        st = g_guard_engine->process_ethercat_l2(std::span<const uint8_t>(pdu, pdu_len));
        matched_rule_id = 0x88A4;
    }

    if (st != motion_guard::GuardStatus::PASS)
    {
        res.verdict = SENTINEL_VERDICT_KERNEL_DROP;
        res.threat_score = 995;
        res.rule_id = matched_rule_id;
        std::strncpy(res.threat_name, motion_guard::status_to_string(st).data(), sizeof(res.threat_name) - 1);

        if (g_host)
        {
            if (g_host->log_message)
            {
                std::string msg = "MOTION SAFETY TRIPWIRE: Clamped actuator (" +
                                  std::string(res.threat_name) + ")";
                g_host->log_message(3, "MotionGuard", msg.c_str());
            }
            if (g_host->emit_metric_counter)
            {
                g_host->emit_metric_counter("motion_cbf_airgap_trips", 1);
            }
        }
        return res;
    }

    res.threat_score = static_cast<uint32_t>(std::min<size_t>(packet->length * 2U, 180U));
    return res;
}

static const SentinelPluginDescriptor g_descriptor = {
    .magic = SENTINEL_SDK_MAGIC,
    .sdk_version_major = SENTINEL_SDK_VERSION_MAJOR,
    .sdk_version_minor = SENTINEL_SDK_VERSION_MINOR,
    .tier = SENTINEL_TIER_NATIVE_CPP,
    .plugin_id = "org.aryorithm.plugin.motion_guard",
    .plugin_name = "Robotics EtherCAT, CAN-FD, ShmRing & Surgical TSN Safety Governor",
    .plugin_version = "1.0.0",
    .target_protocol = "ETHERCAT_CANFD_SHMRING_TSN_CBF",
    .default_port = 0,
    .reserved = 0,
    .init_fn = plugin_init,
    .shutdown_fn = plugin_shutdown,
    .dissect_fn = plugin_dissect};

extern "C" ARY_SDK_API const SentinelPluginDescriptor *sentinel_plugin_get_descriptor(void)
{
    return &g_descriptor;
}