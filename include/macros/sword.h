#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Sword Macros                              ║
// ║  W-Tap & Sprint Reset for enhanced knockback            ║
// ╚══════════════════════════════════════════════════════════╝

#include "macros/macro_base.h"
#include <windows.h>

namespace noqwd {

struct SwordSettings {
    // W-Tap
    bool wtap_enabled       = false;
    int  wtap_release_delay = 40;   // ms to release W
    int  wtap_press_delay   = 40;   // ms before re-pressing W
    bool wtap_auto_attack   = true; // auto left-click on wtap

    // Sprint Reset
    bool sprint_reset_enabled = false;
    int  sprint_reset_delay   = 30;
    bool sprint_double_tap    = true; // use ctrl or double-tap W

    // Sword keybind (vk code to switch to sword)
    int  sword_key = 0;

    // Jitter
    int jitter_min = 3;
    int jitter_max = 7;
};

// ─── W-Tap ──────────────────────────────────────────────
class WTap : public MacroBase {
public:
    WTap(SwordSettings& settings);
    void execute() override;
private:
    SwordSettings& settings_;
};

// ─── Sprint Reset ───────────────────────────────────────
class SprintReset : public MacroBase {
public:
    SprintReset(SwordSettings& settings);
    void execute() override;
private:
    SwordSettings& settings_;
};

} // namespace noqwd
