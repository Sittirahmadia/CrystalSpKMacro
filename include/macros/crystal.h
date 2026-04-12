#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Crystal PVP Macros                        ║
// ║  HC  = Hit Crystal (place obsidian → detonate crystal)  ║
// ║  SA  = Single Anchor (place → charge → detonate)        ║
// ║  DA  = Double Anchor (two rapid anchor detonations)     ║
// ║  AP  = Anchor Pearl  (detonate anchor → throw pearl)    ║
// ╚══════════════════════════════════════════════════════════╝

#include "macros/macro_base.h"
#include <windows.h>

namespace noqwd {

// ─── Settings shared by crystal macros ──────────────────
struct CrystalSettings {
    // HC
    bool hc_enabled        = false;
    int  hc_place_delay    = 50;   // ms between obsidian place & crystal break
    int  hc_break_delay    = 50;
    int  hc_obsidian_key   = 0;    // keybind for obsidian (vk code, 0 = unbound)
    int  hc_crystal_key    = 0;    // keybind for crystal

    // SA
    bool sa_enabled        = false;
    int  sa_place_delay    = 40;
    int  sa_charge_delay   = 40;
    int  sa_detonate_delay = 40;
    int  sa_anchor_key     = 0;    // keybind for anchor
    int  sa_glowstone_key  = 0;    // keybind for glowstone

    // DA
    bool da_enabled        = false;
    int  da_first_delay    = 50;
    int  da_second_delay   = 75;
    int  da_between_delay  = 75;
    int  da_anchor_key     = 0;
    int  da_glowstone_key  = 0;

    // AP
    bool ap_enabled        = false;
    int  ap_detonate_delay = 50;
    int  ap_pearl_delay    = 50;
    int  ap_anchor_key     = 0;
    int  ap_glowstone_key  = 0;
    int  ap_pearl_key      = 0;    // keybind for ender pearl

    // Global jitter
    int jitter_min = 3;
    int jitter_max = 7;
};

// ─── Hit Crystal ────────────────────────────────────────
class HitCrystal : public MacroBase {
public:
    HitCrystal(CrystalSettings& settings);
    void execute() override;
private:
    CrystalSettings& settings_;
};

// ─── Single Anchor ──────────────────────────────────────
class SingleAnchor : public MacroBase {
public:
    SingleAnchor(CrystalSettings& settings);
    void execute() override;
private:
    CrystalSettings& settings_;
};

// ─── Double Anchor ──────────────────────────────────────
class DoubleAnchor : public MacroBase {
public:
    DoubleAnchor(CrystalSettings& settings);
    void execute() override;
private:
    CrystalSettings& settings_;
};

// ─── Anchor Pearl ───────────────────────────────────────
class AnchorPearl : public MacroBase {
public:
    AnchorPearl(CrystalSettings& settings);
    void execute() override;
private:
    CrystalSettings& settings_;
};

} // namespace noqwd
