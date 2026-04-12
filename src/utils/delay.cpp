// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Humanised Delay System                    ║
// ╚══════════════════════════════════════════════════════════╝

#include "utils/delay.h"
#include <thread>
#include <chrono>
#include <windows.h>

namespace noqwd {

std::mt19937& Delay::rng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

int Delay::jittered(int base_ms, int jitter_min, int jitter_max) {
    if (base_ms <= 0) return 0;

    std::uniform_int_distribution<int> jitter_range(jitter_min, jitter_max);
    std::uniform_int_distribution<int> sign_dist(0, 1);

    int jitter = jitter_range(rng());
    if (sign_dist(rng()) == 0) jitter = -jitter;

    int result = base_ms + jitter;
    return (result < 1) ? 1 : result;
}

void Delay::sleep(int base_ms, int jitter_min, int jitter_max) {
    int actual = jittered(base_ms, jitter_min, jitter_max);
    if (actual <= 0) return;

    // Use a combination of Sleep + spin-wait for accuracy
    if (actual > 2) {
        // Sleep for most of the duration (coarse)
        std::this_thread::sleep_for(std::chrono::milliseconds(actual - 1));
    }

    // Spin-wait for the remaining sub-ms precision
    auto target = std::chrono::high_resolution_clock::now() +
                  std::chrono::milliseconds(1);
    while (std::chrono::high_resolution_clock::now() < target) {
        _mm_pause();  // CPU hint — reduces power in spin loop
    }
}

void Delay::precision_sleep_us(int microseconds) {
    if (microseconds <= 0) return;

    auto target = std::chrono::high_resolution_clock::now() +
                  std::chrono::microseconds(microseconds);

    // Coarse sleep if > 1ms
    if (microseconds > 1500) {
        std::this_thread::sleep_for(
            std::chrono::microseconds(microseconds - 1000));
    }

    // Spin-wait for remaining time
    while (std::chrono::high_resolution_clock::now() < target) {
        _mm_pause();
    }
}

} // namespace noqwd
