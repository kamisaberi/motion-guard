// File: /home/kami/motion-guard/tests/test_sentinel_plugin.cpp

#include "sentinel/sdk/abi.hpp"
#include <cassert>
#include <cstring>
#include <dlfcn.h>
#include <filesystem>
#include <iostream>
#include <vector>

static int g_logs = 0;
static int g_metrics = 0;

static void mock_log(int level, const char *sender, const char *msg)
{
    std::cout << "  [HOST_LOG level=" << level << " sender=" << sender << "] " << msg << "\n";
    g_logs++;
}

static void mock_metric(const char *name, uint64_t delta)
{
    std::cout << "  [HOST_METRIC] " << name << " (+" << delta << ")\n";
    g_metrics += static_cast<int>(delta);
}

int main()
{
    std::cout << "[TEST] blackbox-sentinel 3-Bus Motion-Guard Plugin ABI Integration...\n";

    const char *plugin_path = std::filesystem::exists("./motion_guard_dissector_plugin.so")
                                  ? "./motion_guard_dissector_plugin.so"
                                  : "/usr/local/lib/blackbox/motion_guard_dissector_plugin.so";

    void *handle = dlopen(plugin_path, RTLD_NOW | RTLD_LOCAL);
    if (!handle)
    {
        std::cerr << "[-] dlopen failed: " << dlerror() << "\n";
        return 1;
    }

    using GetDescFn = const SentinelPluginDescriptor *(*)(void);
    auto get_desc = reinterpret_cast<GetDescFn>(dlsym(handle, "sentinel_plugin_get_descriptor"));
    assert(get_desc != nullptr);
    const SentinelPluginDescriptor *desc = get_desc();
    assert(desc && desc->magic == SENTINEL_SDK_MAGIC);
    assert(std::string(desc->plugin_id) == "org.aryorithm.plugin.motion_guard");

    SentinelHostInterface host{};
    host.engine_version = 1;
    host.log_message = mock_log;
    host.emit_metric_counter = mock_metric;
    assert(desc->init_fn(&host) == 0);
    assert(g_logs >= 1);

    // 1. Bus A: Valid EtherCAT 0x88A4 -> PASS
    std::vector<uint8_t> ecat_ok(24, 0);
    const uint16_t hdr = static_cast<uint16_t>((1U << 12) | 22U);
    std::memcpy(&ecat_ok[0], &hdr, 2);
    float accel = 4.0f, rad = 300.0f;
    std::memcpy(&ecat_ok[12], &accel, 4);
    std::memcpy(&ecat_ok[16], &rad, 4);
    uint16_t wkc = 1;
    std::memcpy(&ecat_ok[20], &wkc, 2);

    SentinelRawPacket p1{};
    p1.data = ecat_ok.data();
    p1.length = ecat_ok.size();
    p1.network_proto = 0x88A4;
    p1.reserved = 0;
    assert(desc->dissect_fn(&p1).verdict == SENTINEL_VERDICT_PASS);

    // 2. Bus B (reserved=1): Rogue CAN-FD steering override -> KERNEL_DROP
    std::vector<uint8_t> can_bad(16, 0);
    uint32_t cid = 0x201;
    std::memcpy(&can_bad[0], &cid, 4);
    can_bad[4] = 8;
    float steer = 72.0f, torque = 120.0f;
    std::memcpy(&can_bad[8], &steer, 4);
    std::memcpy(&can_bad[12], &torque, 4);

    SentinelRawPacket p2{};
    p2.data = can_bad.data();
    p2.length = can_bad.size();
    p2.network_proto = 0x000C;
    p2.transport_proto = 0xCA;
    p2.reserved = 1; // IngressBusType::SOCKET_CAN

    SentinelDissectorResult r2 = desc->dissect_fn(&p2);
    assert(r2.verdict == SENTINEL_VERDICT_KERNEL_DROP);
    assert(r2.rule_id == 0x0CA4);
    assert(g_metrics >= 1);

    desc->shutdown_fn();
    dlclose(handle);
    std::cout << ">>> RESULT: SUCCESS (3-Bus Motion-Guard Plugin Verified) <<<\n";
    return 0;
}
