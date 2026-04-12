#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Humanised Delay System                    ║
// ╚══════════════════════════════════════════════════════════╝

#include <cstdint>
#include <random>

namespace noqwd {

class Delay {
public:
    // Sleep for `base_ms` with a random jitter of ±[jitter_min, jitter_max]
    static void sleep(int base_ms, int jitter_min = 3, int jitter_max = 7);

    // Get a jittered value without sleeping (useful for reporting)
    static int jittered(int base_ms, int jitter_min = 3, int jitter_max = 7);

    // Precision sleep using spin-wait for sub-millisecond accuracy
    static void precision_sleep_us(int microseconds);

private:
    static std::mt19937& rng();
};

} // namespace noqwd
