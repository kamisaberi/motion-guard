// File: /home/kami/motion-guard/include/motion_guard/defense/motion_airgap.hpp

#pragma once

#include <atomic>

namespace motion_guard::defense {

class MotionAirGap {
public:
    void trip() noexcept { tripped_.store(true, std::memory_order_release); }
    void reset() noexcept { tripped_.store(false, std::memory_order_release); }
    [[nodiscard]] bool is_tripped() const noexcept { return tripped_.load(std::memory_order_acquire); }

private:
    std::atomic<bool> tripped_{false};
};

} // namespace motion_guard::defense
