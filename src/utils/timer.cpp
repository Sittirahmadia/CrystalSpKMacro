// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — High-Resolution Timer                     ║
// ╚══════════════════════════════════════════════════════════╝

#include "utils/timer.h"

namespace noqwd {

Timer::Timer() : start_(std::chrono::high_resolution_clock::now()) {}

void Timer::reset() {
    start_ = std::chrono::high_resolution_clock::now();
}

double Timer::elapsed_ms() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(now - start_).count();
}

double Timer::elapsed_us() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::micro>(now - start_).count();
}

bool Timer::has_elapsed(double ms) const {
    return elapsed_ms() >= ms;
}

} // namespace noqwd
