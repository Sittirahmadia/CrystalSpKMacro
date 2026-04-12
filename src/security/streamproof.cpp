// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Streamproof Module                        ║
// ║  Hides overlay from OBS, Discord, and screen capture    ║
// ╚══════════════════════════════════════════════════════════╝

#include "security/streamproof.h"

// WDA_EXCLUDEFROMCAPTURE may not be defined in older SDK headers
#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

#ifndef WDA_NONE
#define WDA_NONE 0x00000000
#endif

namespace noqwd {

bool Streamproof::enabled_ = false;

bool Streamproof::enable(HWND hwnd) {
    if (!hwnd) return false;

    // SetWindowDisplayAffinity with WDA_EXCLUDEFROMCAPTURE
    // This makes the window invisible to screen capture APIs
    // while still being visible to the user on their monitor.
    BOOL result = SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
    if (result) {
        enabled_ = true;
        return true;
    }

    // Fallback: try WDA_MONITOR (older Windows 10 versions)
    // This shows black in capture instead of the window content
    result = SetWindowDisplayAffinity(hwnd, 0x00000001);  // WDA_MONITOR
    if (result) {
        enabled_ = true;
        return true;
    }

    return false;
}

bool Streamproof::disable(HWND hwnd) {
    if (!hwnd) return false;

    BOOL result = SetWindowDisplayAffinity(hwnd, WDA_NONE);
    if (result) {
        enabled_ = false;
        return true;
    }
    return false;
}

bool Streamproof::toggle(HWND hwnd) {
    return enabled_ ? disable(hwnd) : enable(hwnd);
}

bool Streamproof::is_enabled() {
    return enabled_;
}

} // namespace noqwd
