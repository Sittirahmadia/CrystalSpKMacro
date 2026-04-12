#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Configuration & Global Constants          ║
// ║  Version 1.4.0                                          ║
// ╚══════════════════════════════════════════════════════════╝

#include <string>
#include <cstdint>
#include <windows.h>

namespace noqwd {

// ─── Version Info ────────────────────────────────────────
constexpr const char* APP_NAME    = "noqwdmacro";
constexpr const char* APP_VERSION = "v1.4.0";
constexpr const char* APP_AUTHOR  = "noqwd";

// ─── Window Settings ────────────────────────────────────
constexpr int    WINDOW_WIDTH      = 820;
constexpr int    WINDOW_HEIGHT     = 540;
constexpr float  SIDEBAR_WIDTH     = 180.0f;
constexpr float  STATUSBAR_HEIGHT  = 32.0f;

// ─── Timing Defaults (ms) ───────────────────────────────
namespace defaults {
    // Crystal PVP
    constexpr int HC_PLACE_DELAY       = 50;
    constexpr int HC_BREAK_DELAY       = 50;
    constexpr int SA_PLACE_DELAY       = 40;
    constexpr int SA_CHARGE_DELAY      = 40;
    constexpr int SA_DETONATE_DELAY    = 40;
    constexpr int DA_FIRST_DELAY       = 50;
    constexpr int DA_SECOND_DELAY      = 75;
    constexpr int DA_BETWEEN_DELAY     = 75;
    constexpr int AP_DETONATE_DELAY    = 50;
    constexpr int AP_PEARL_DELAY       = 50;

    // Mace
    constexpr int MACE_ATTACK_DELAY    = 50;
    constexpr int WIND_CHARGE_DELAY    = 50;

    // Sword
    constexpr int WTAP_RELEASE_DELAY   = 40;
    constexpr int WTAP_PRESS_DELAY     = 40;
    constexpr int SPRINT_RESET_DELAY   = 30;

    // Randomisation
    constexpr int JITTER_MIN           = 3;
    constexpr int JITTER_MAX           = 7;
}

// ─── Delay Limits ───────────────────────────────────────
constexpr int DELAY_MIN = 1;
constexpr int DELAY_MAX = 500;

// ─── Mouse Side Buttons (for macro triggers) ────────────
// XBUTTON1 = Mouse4 (back), XBUTTON2 = Mouse5 (forward)
constexpr int MOUSE_SIDE_1 = VK_XBUTTON1;   // 0x05
constexpr int MOUSE_SIDE_2 = VK_XBUTTON2;   // 0x06

} // namespace noqwd
