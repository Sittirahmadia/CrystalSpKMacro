// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Mace Macro Implementations                ║
// ║  All item switching uses direct keybinds                ║
// ╚══════════════════════════════════════════════════════════╝

#include "macros/mace.h"
#include "input/input_simulator.h"
#include "utils/delay.h"

namespace noqwd {

// ═════════════════════════════════════════════════════════
//  Mace Attack
//  Sequence: Press mace key → precise-timed left-click
// ═════════════════════════════════════════════════════════
MaceAttack::MaceAttack(MaceSettings& settings)
    : MacroBase("Mace Attack"), settings_(settings) {}

void MaceAttack::execute() {
    if (!enabled_ || !settings_.mace_enabled) return;
    if (settings_.mace_key == 0) return;

    int jmin = settings_.jitter_min;
    int jmax = settings_.jitter_max;

    // Switch to mace
    InputSimulator::key_press(static_cast<WORD>(settings_.mace_key));
    Delay::sleep(settings_.mace_attack_delay, jmin, jmax);

    // Attack
    InputSimulator::mouse_left_click();
}

// ═════════════════════════════════════════════════════════
//  Wind Charge Combo
//  Sequence: Press wind charge key → throw (right-click) →
//            wait → press mace key → attack (left-click)
// ═════════════════════════════════════════════════════════
WindChargeCombo::WindChargeCombo(MaceSettings& settings)
    : MacroBase("Wind Charge Combo"), settings_(settings) {}

void WindChargeCombo::execute() {
    if (!enabled_ || !settings_.wind_enabled) return;
    if (settings_.wind_charge_key == 0 || settings_.mace_combo_key == 0) return;

    int jmin = settings_.jitter_min;
    int jmax = settings_.jitter_max;

    // 1. Switch to wind charge and throw
    InputSimulator::key_press(static_cast<WORD>(settings_.wind_charge_key));
    Delay::sleep(settings_.wind_charge_delay, jmin, jmax);
    InputSimulator::mouse_right_click();

    // 2. Wait for wind charge travel, then swap to mace
    Delay::sleep(settings_.wind_swap_delay, jmin, jmax);
    InputSimulator::key_press(static_cast<WORD>(settings_.mace_combo_key));

    // 3. Attack with mace while boosted
    Delay::sleep(settings_.wind_attack_delay, jmin, jmax);
    InputSimulator::mouse_left_click();
}

} // namespace noqwd
