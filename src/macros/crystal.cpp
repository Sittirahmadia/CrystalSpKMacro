// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Crystal PVP Macro Implementations         ║
// ║  All item switching uses direct keybinds (no slot nums) ║
// ╚══════════════════════════════════════════════════════════╝

#include "macros/crystal.h"
#include "input/input_simulator.h"
#include "utils/delay.h"

namespace noqwd {

// ═════════════════════════════════════════════════════════
//  Hit Crystal (HC)
//  Sequence: Press obsidian key → place (right-click) →
//            press crystal key → break (left-click)
// ═════════════════════════════════════════════════════════
HitCrystal::HitCrystal(CrystalSettings& settings)
    : MacroBase("Hit Crystal"), settings_(settings) {}

void HitCrystal::execute() {
    if (!enabled_ || !settings_.hc_enabled) return;
    if (settings_.hc_obsidian_key == 0 || settings_.hc_crystal_key == 0) return;

    int jmin = settings_.jitter_min;
    int jmax = settings_.jitter_max;

    // 1. Press obsidian keybind and place
    InputSimulator::key_press(static_cast<WORD>(settings_.hc_obsidian_key));
    Delay::sleep(settings_.hc_place_delay, jmin, jmax);

    InputSimulator::mouse_right_click();
    Delay::sleep(settings_.hc_place_delay, jmin, jmax);

    // 2. Press crystal keybind and break
    InputSimulator::key_press(static_cast<WORD>(settings_.hc_crystal_key));
    Delay::sleep(settings_.hc_break_delay, jmin, jmax);

    InputSimulator::mouse_left_click();
}

// ═════════════════════════════════════════════════════════
//  Single Anchor (SA)
//  Sequence: Press anchor key → place (right-click) →
//            press glowstone key → charge (right-click) →
//            press anchor key → detonate (right-click)
// ═════════════════════════════════════════════════════════
SingleAnchor::SingleAnchor(CrystalSettings& settings)
    : MacroBase("Single Anchor"), settings_(settings) {}

void SingleAnchor::execute() {
    if (!enabled_ || !settings_.sa_enabled) return;
    if (settings_.sa_anchor_key == 0 || settings_.sa_glowstone_key == 0) return;

    int jmin = settings_.jitter_min;
    int jmax = settings_.jitter_max;

    // 1. Place Respawn Anchor
    InputSimulator::key_press(static_cast<WORD>(settings_.sa_anchor_key));
    Delay::sleep(settings_.sa_place_delay, jmin, jmax);
    InputSimulator::mouse_right_click();
    Delay::sleep(settings_.sa_place_delay, jmin, jmax);

    // 2. Switch to Glowstone and charge
    InputSimulator::key_press(static_cast<WORD>(settings_.sa_glowstone_key));
    Delay::sleep(settings_.sa_charge_delay, jmin, jmax);
    InputSimulator::mouse_right_click();
    Delay::sleep(settings_.sa_charge_delay, jmin, jmax);

    // 3. Switch back to anchor and detonate
    InputSimulator::key_press(static_cast<WORD>(settings_.sa_anchor_key));
    Delay::sleep(settings_.sa_detonate_delay, jmin, jmax);
    InputSimulator::mouse_right_click();
}

// ═════════════════════════════════════════════════════════
//  Double Anchor (DA)
//  Sequence: Place first anchor → charge → detonate →
//            (75ms default) → place second → charge → detonate
// ═════════════════════════════════════════════════════════
DoubleAnchor::DoubleAnchor(CrystalSettings& settings)
    : MacroBase("Double Anchor"), settings_(settings) {}

void DoubleAnchor::execute() {
    if (!enabled_ || !settings_.da_enabled) return;
    if (settings_.da_anchor_key == 0 || settings_.da_glowstone_key == 0) return;

    int jmin = settings_.jitter_min;
    int jmax = settings_.jitter_max;

    auto do_anchor_cycle = [&](int place_delay, int charge_delay) {
        // Place anchor
        InputSimulator::key_press(static_cast<WORD>(settings_.da_anchor_key));
        Delay::sleep(place_delay, jmin, jmax);
        InputSimulator::mouse_right_click();
        Delay::sleep(place_delay, jmin, jmax);

        // Charge with glowstone
        InputSimulator::key_press(static_cast<WORD>(settings_.da_glowstone_key));
        Delay::sleep(charge_delay, jmin, jmax);
        InputSimulator::mouse_right_click();
        Delay::sleep(charge_delay, jmin, jmax);

        // Detonate
        InputSimulator::key_press(static_cast<WORD>(settings_.da_anchor_key));
        Delay::sleep(charge_delay, jmin, jmax);
        InputSimulator::mouse_right_click();
    };

    // First anchor
    do_anchor_cycle(settings_.da_first_delay, settings_.da_first_delay);

    // Gap between anchors
    Delay::sleep(settings_.da_between_delay, jmin, jmax);

    // Second anchor
    do_anchor_cycle(settings_.da_second_delay, settings_.da_second_delay);
}

// ═════════════════════════════════════════════════════════
//  Anchor Pearl (AP)
//  Sequence: Place & charge anchor → detonate →
//            press pearl key → throw pearl
// ═════════════════════════════════════════════════════════
AnchorPearl::AnchorPearl(CrystalSettings& settings)
    : MacroBase("Anchor Pearl"), settings_(settings) {}

void AnchorPearl::execute() {
    if (!enabled_ || !settings_.ap_enabled) return;
    if (settings_.ap_anchor_key == 0 || settings_.ap_glowstone_key == 0 ||
        settings_.ap_pearl_key == 0) return;

    int jmin = settings_.jitter_min;
    int jmax = settings_.jitter_max;

    // 1. Place anchor
    InputSimulator::key_press(static_cast<WORD>(settings_.ap_anchor_key));
    Delay::sleep(settings_.ap_detonate_delay, jmin, jmax);
    InputSimulator::mouse_right_click();
    Delay::sleep(settings_.ap_detonate_delay, jmin, jmax);

    // 2. Charge with glowstone
    InputSimulator::key_press(static_cast<WORD>(settings_.ap_glowstone_key));
    Delay::sleep(settings_.ap_detonate_delay, jmin, jmax);
    InputSimulator::mouse_right_click();
    Delay::sleep(settings_.ap_detonate_delay, jmin, jmax);

    // 3. Detonate
    InputSimulator::key_press(static_cast<WORD>(settings_.ap_anchor_key));
    Delay::sleep(settings_.ap_detonate_delay, jmin, jmax);
    InputSimulator::mouse_right_click();

    // 4. Switch to pearl and throw
    Delay::sleep(settings_.ap_pearl_delay, jmin, jmax);
    InputSimulator::key_press(static_cast<WORD>(settings_.ap_pearl_key));
    Delay::sleep(settings_.ap_pearl_delay, jmin, jmax);
    InputSimulator::mouse_right_click();
}

} // namespace noqwd
