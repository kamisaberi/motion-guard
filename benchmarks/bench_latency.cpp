// File: /home/kami/motion-guard/benchmarks/bench_latency.cpp

#include "motion_guard/motion_guard.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <numeric>
#include <vector>

int main()
{
    motion_guard::core::TscClock::calibrate();
    motion_guard::pipeline::GuardEngine engine;

    constexpr size_t ITERATIONS = 100'000;
    std::vector<double> samples_ns;
    samples_ns.reserve(ITERATIONS);

    std::vector<uint8_t> ecat(24, 0);
    const uint16_t hdr = static_cast<uint16_t>((1U << 12) | 22U);
    std::memcpy(&ecat[0], &hdr, 2);
    float accel = 4.2f, radius = 320.0f;
    std::memcpy(&ecat[12], &accel, 4);
    std::memcpy(&ecat[16], &radius, 4);
    uint16_t wkc = 1;
    std::memcpy(&ecat[20], &wkc, 2);

    for (size_t i = 0; i < ITERATIONS; ++i)
    {
        const uint64_t t0 = motion_guard::core::TscClock::rdtsc();
        const auto st = engine.process_ethercat_l2(ecat);
        const uint64_t t1 = motion_guard::core::TscClock::rdtsc();
        if (st == motion_guard::GuardStatus::PASS)
        {
            samples_ns.push_back(motion_guard::core::TscClock::cycles_to_nanoseconds(t1 - t0));
        }
    }

    std::sort(samples_ns.begin(), samples_ns.end());
    const double sum = std::accumulate(samples_ns.begin(), samples_ns.end(), 0.0);
    const double mean = sum / static_cast<double>(samples_ns.size());
    const double p99 = samples_ns[(samples_ns.size() * 99) / 100];

    std::cout << "================================================================================\n";
    std::cout << "  MOTION-GUARD HARDWARE LATENCY BENCHMARK (100,000 ETHERCAT 0x88A4 FRAMES)\n";
    std::cout << "================================================================================\n";
    std::cout << "  TSC Frequency : " << motion_guard::core::TscClock::ghz() << " GHz\n";
    std::cout << "  Mean Latency  : " << mean << " ns\n";
    std::cout << "  P99 Latency   : " << p99 << " ns\n";
    std::cout << "  SLA Target    : < 120.0 ns -> " << (mean < 120.0 ? "PASS" : "FAIL") << "\n";
    std::cout << "================================================================================\n";
    return (mean < 120.0) ? 0 : 1;
}
