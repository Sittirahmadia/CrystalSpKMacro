// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Sword Macro Implementations               ║
// ║  All item switching uses direct keybinds                ║
// ╚══════════════════════════════════════════════════════════╝

#include "macros/sword.h"
#include "input/input_simulator.h"
#include "utils/delay.h"

namespace noqwd {

// ═════════════════════════════════════════════════════════
//  W-Tap
//  Technique: Briefly release W key during attack to reset
//  sprint status, resulting in increased knockback.
//  Sequence: Left-click → release W → wait → re-press W
// ═════════════════════════════════════════════════════════
WTap::WTap(SwordSettings& settings)
    : MacroBase("W-Tap"), settings_(settings) {}

void WTap::execute() {
    if (!enabled_ || !settings_.wtap_enabled) return;

    int jmin = settings_.jitter_min;
    int jmax = settings_.jitter_max;

    // Optionally switch to sword first
    if (settings_.sword_key != 0) {
        InputSimulator::key_press(static_cast<WORD>(settings_.sword_key));
        Delay::precision_sleep_us(500);
    }

    // Attack
    if (settings_.wtap_auto_attack) {
        InputSimulator::mouse_left_click();
        Delay::precision_sleep_us(300);
    }

    // W-Tap: release W, wait, re-press
    InputSimulator::key_up(0x57);  // 'W'
    Delay::sleep(settings_.wtap_release_delay, jmin, jmax);

    InputSimulator::key_down(0x57);
    Delay::sleep(settings_.wtap_press_delay, jmin, jmax);
}

// ═════════════════════════════════════════════════════════
//  Sprint Reset
//  Technique: Toggle sprint key to reset sprint status
//  without releasing movement keys.
// ═════════════════════════════════════════════════════════
SprintReset::SprintReset(SwordSettings& settings)
    : MacroBase("Sprint Reset"), settings_(settings) {}

void SprintReset::execute() {
    if (!enabled_ || !settings_.sprint_reset_enabled) return;

    int jmin = settings_.jitter_min;
    int jmax = settings_.jitter_max;

    if (settings_.sprint_double_tap) {
        // Double-tap W method
        InputSimulator::key_up(0x57);
        Delay::sleep(settings_.sprint_reset_delay, jmin, jmax);
        InputSimulator::key_down(0x57);
        Delay::precision_sleep_us(200);
        InputSimulator::key_up(0x57);
        Delay::precision_sleep_us(200);
        InputSimulator::key_down(0x57);
    } else {
        // Ctrl toggle method
        InputSimulator::key_up(VK_LCONTROL);
        Delay::sleep(settings_.sprint_reset_delay, jmin, jmax);
        InputSimulator::key_down(VK_LCONTROL);
    }

    // Attack after sprint reset
    Delay::sleep(settings_.sprint_reset_delay, jmin, jmax);
    InputSimulator::mouse_left_click();
}

} // namespace noqwd
