#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Mace Macros                               ║
// ║  Mace Attack & Wind Charge Combo                        ║
// ╚══════════════════════════════════════════════════════════╝

#include "macros/macro_base.h"
#include <windows.h>

namespace noqwd {

struct MaceSettings {
    // Mace Attack
    bool mace_enabled       = false;
    int  mace_attack_delay  = 50;
    int  mace_key           = 0;    // keybind to switch to mace (vk code)

    // Wind Charge Combo
    bool wind_enabled       = false;
    int  wind_charge_delay  = 50;
    int  wind_swap_delay    = 40;   // delay between wind charge throw → mace swap
    int  wind_attack_delay  = 50;   // delay after swap → attack
    int  wind_charge_key    = 0;    // keybind for wind charge item
    int  mace_combo_key     = 0;    // keybind for mace in combo

    // Jitter
    int jitter_min = 3;
    int jitter_max = 7;
};

// ─── Mace Attack ────────────────────────────────────────
class MaceAttack : public MacroBase {
public:
    MaceAttack(MaceSettings& settings);
    void execute() override;
private:
    MaceSettings& settings_;
};

// ─── Wind Charge Combo ─────────────────────────────────
class WindChargeCombo : public MacroBase {
public:
    WindChargeCombo(MaceSettings& settings);
    void execute() override;
private:
    MaceSettings& settings_;
};

} // namespace noqwd
