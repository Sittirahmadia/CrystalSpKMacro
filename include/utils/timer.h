#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — High-Resolution Timer                     ║
// ╚══════════════════════════════════════════════════════════╝

#include <chrono>

namespace noqwd {

class Timer {
public:
    Timer();

    void   reset();
    double elapsed_ms() const;
    double elapsed_us() const;
    bool   has_elapsed(double ms) const;

private:
    std::chrono::high_resolution_clock::time_point start_;
};

} // namespace noqwd
