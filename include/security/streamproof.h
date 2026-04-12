#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Streamproof Module                        ║
// ║  Uses SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE)  ║
// ║  to hide overlay from OBS / Discord / screen capture    ║
// ╚══════════════════════════════════════════════════════════╝

#include <windows.h>

namespace noqwd {

class Streamproof {
public:
    // Apply streamproof to a given HWND
    static bool enable(HWND hwnd);

    // Remove streamproof (make window visible to capture again)
    static bool disable(HWND hwnd);

    // Toggle
    static bool toggle(HWND hwnd);

    // Query state
    static bool is_enabled();

private:
    static bool enabled_;
};

} // namespace noqwd
