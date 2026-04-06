// ─────────────────────────────────────────────────────────────────────────────
//  macros.cpp — All 21 macro implementations using native Win32 SendInput
//
//  Every macro checks g_cancel between steps and exits early if set.
//  One-shot macros run on the calling thread (engine dispatches to a worker).
//  Hold-to-run macros (FXP, AC) manage their own background threads.
// ─────────────────────────────────────────────────────────────────────────────

#include "macros.h"
#include "input.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <set>

namespace macros {

std::atomic<bool> g_cancel{false};

// Track which one-shot macros are currently running
static std::mutex g_runMtx;
static std::set<std::string> g_running;

struct RunGuard {
    std::string id;
    RunGuard(const std::string& i) : id(i) {
        std::lock_guard<std::mutex> lk(g_runMtx);
        g_running.insert(id);
    }
    ~RunGuard() {
        std::lock_guard<std::mutex> lk(g_runMtx);
        g_running.erase(id);
    }
};

bool isRunning(const std::string& id) {
    std::lock_guard<std::mutex> lk(g_runMtx);
    return g_running.count(id) > 0;
}

// ── Helpers ─────────────────────────────────────────────────────────────────

#define CHECK if (g_cancel.load()) return
#define WAIT(ms) do { preciseSleep(ms); if (g_cancel.load()) return; } while(0)

// ── SA — Single Anchor ──────────────────────────────────────────────────────
// Concurrent slot-switch + right-click per step via batched SendInput.
// Sequence: anchor slot+rclick → glowstone slot+rclick → detonate slot+rclick
void runSA(uint16_t anchorVk, uint16_t glowstoneVk, uint16_t explodeVk, int delay) {
    RunGuard rg("sa");
    int d = std::max(0, delay);
    uint16_t detVk = explodeVk ? explodeVk : anchorVk;

    focusMc();
    slotClick(anchorVk, SLOT_HOLD_MS);    WAIT(d);
    slotClick(glowstoneVk, SLOT_HOLD_MS); WAIT(d);
    slotClick(detVk, SLOT_HOLD_MS);
}

// ── DA — Double Anchor (Airplace) ───────────────────────────────────────────
// Hold right-click the entire time, only switch hotbar slots.
// anchor → glowstone → anchor(detonate+airplace) → glowstone → anchor(detonate)
void runDA(uint16_t anchorVk, uint16_t glowstoneVk, int delay) {
    RunGuard rg("da");
    int d = std::max(0, delay);

    focusMc();
    preciseSleep(2);

    mouseDown(true); // hold right-click

    keyPress(anchorVk, SLOT_HOLD_MS);    WAIT(d); CHECK;
    keyPress(glowstoneVk, SLOT_HOLD_MS); WAIT(d); CHECK;
    keyPress(anchorVk, SLOT_HOLD_MS);    WAIT(d); CHECK; // detonate + airplace
    keyPress(glowstoneVk, SLOT_HOLD_MS); WAIT(d); CHECK;
    keyPress(anchorVk, SLOT_HOLD_MS);    WAIT(d);        // detonate second

    mouseUp(true); // release right-click
}

// ── AP — Anchor Pearl ───────────────────────────────────────────────────────
// SA cycle → throw pearl → swap to end key
void runAP(uint16_t anchorVk, uint16_t glowstoneVk, uint16_t explodeVk,
           uint16_t pearlVk, uint16_t totemVk, int delay) {
    RunGuard rg("ap");
    int d = std::max(0, delay);
    uint16_t detVk = explodeVk ? explodeVk : anchorVk;
    uint16_t endVk = explodeVk ? explodeVk : (totemVk ? totemVk : keyToVK("9"));

    focusMc();
    slotClick(anchorVk, SLOT_HOLD_MS);    WAIT(d);
    slotClick(glowstoneVk, SLOT_HOLD_MS); WAIT(d);
    slotClick(detVk, SLOT_HOLD_MS);       WAIT(d);
    keyPress(pearlVk, SLOT_HOLD_MS);
    WAIT(12);
    mouseClick(true, CLICK_HOLD_MS); // throw pearl
    WAIT(std::max(10, d));
    keyPress(endVk, SLOT_HOLD_MS);
}

// ── HC — Hit Crystal ────────────────────────────────────────────────────────
// Place obsidian + place crystal + hit crystal (left-click to detonate)
void runHC(uint16_t obsidianVk, uint16_t crystalVk, int delay) {
    RunGuard rg("hc");
    int d = std::max(0, delay);

    focusMc();
    preciseSleep(2);

    slotClick(obsidianVk, SLOT_HOLD_MS); CHECK; WAIT(d);
    slotClick(crystalVk, SLOT_HOLD_MS);  CHECK; WAIT(d);
    mouseClick(false, CLICK_HOLD_MS); // left-click to detonate
}

// ── SHC — Slow Hit Crystal ──────────────────────────────────────────────────
// Same as HC but with longer delays for high-latency servers.
// Uses sequential press → wait → click instead of concurrent slotClick.
void runSHC(uint16_t obsidianVk, uint16_t crystalVk, int delay) {
    RunGuard rg("shc");
    int d = std::max(20, delay);

    focusMc();
    preciseSleep(2);

    // Place obsidian (sequential for reliability)
    keyPress(obsidianVk, KEY_HOLD_MS); WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);   WAIT(d);

    // Place crystal
    keyPress(crystalVk, KEY_HOLD_MS);  WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);   WAIT(d);

    // Hit crystal to detonate
    mouseClick(false, CLICK_HOLD_MS);
}

// ── KP — Key Pearl ──────────────────────────────────────────────────────────
// Switch to pearl → throw → switch back
void runKP(uint16_t pearlVk, uint16_t returnVk, int delay) {
    RunGuard rg("kp");
    int d = std::max(0, delay);

    preciseSleep(30);
    focusMc();
    keyPress(pearlVk, KEY_HOLD_MS);   WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);  WAIT(d);
    keyPress(returnVk, KEY_HOLD_MS);
}

// ── IDH — Inventory D-Hand ──────────────────────────────────────────────────
// Switch to totem slot → open inventory
void runIDH(uint16_t inventoryVk, uint16_t totemVk, int delay) {
    RunGuard rg("idh");
    int d = std::max(0, delay);

    focusMc();
    keyPress(totemVk, KEY_HOLD_MS);     WAIT(d);
    if (inventoryVk) keyPress(inventoryVk, KEY_HOLD_MS);
}

// ── OHT — Offhand Totem ─────────────────────────────────────────────────────
// Switch to totem → swap hands
void runOHT(uint16_t totemVk, uint16_t swapVk, int delay) {
    RunGuard rg("oht");
    int d = std::max(0, delay);

    preciseSleep(30);
    focusMc();
    keyPress(totemVk, KEY_HOLD_MS); WAIT(d);
    keyPress(swapVk, KEY_HOLD_MS);
}

// ── ASB — Auto Shield Breaker ───────────────────────────────────────────────
// Swap to axe → left-click (break shield) → swap back to sword
void runASB(uint16_t axeVk, uint16_t swordVk, int delay) {
    RunGuard rg("asb");
    int d = std::max(0, delay);

    preciseSleep(30);
    focusMc();
    keyPress(axeVk, KEY_HOLD_MS);     WAIT(d);
    mouseClick(false, CLICK_HOLD_MS); WAIT(d);
    keyPress(swordVk, KEY_HOLD_MS);
}

// ── ES — Elytra Swap ───────────────────────────────────────────────────────
// Switch to elytra → right-click equip → switch back
void runES(uint16_t elytraVk, uint16_t returnVk, int delay) {
    RunGuard rg("es");
    int d = std::max(0, delay);

    preciseSleep(30);
    focusMc();
    keyPress(elytraVk, SLOT_HOLD_MS);   WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);     WAIT(std::max(12, d));
    keyPress(returnVk, SLOT_HOLD_MS);
}

// ── PC — Pearl Catch ────────────────────────────────────────────────────────
// Throw pearl → throw wind charge
void runPC(uint16_t pearlVk, uint16_t windChargeVk, int delay) {
    RunGuard rg("pc");
    int d = std::max(0, delay);

    preciseSleep(30);
    focusMc();
    keyPress(pearlVk, KEY_HOLD_MS);
    mouseClick(true, CLICK_HOLD_MS); WAIT(d);
    keyPress(windChargeVk, KEY_HOLD_MS);
    mouseClick(true, CLICK_HOLD_MS);
}

// ── SS — Stun Slam ──────────────────────────────────────────────────────────
// Axe hit (break shield) → mace hit
void runSS(uint16_t axeVk, uint16_t maceVk, int delay) {
    RunGuard rg("ss");
    int d = std::max(0, delay);

    preciseSleep(30);
    focusMc();
    keyPress(axeVk, KEY_HOLD_MS);
    mouseClick(false, CLICK_HOLD_MS); WAIT(d);
    keyPress(maceVk, KEY_HOLD_MS);
    mouseClick(false, CLICK_HOLD_MS);
}

// ── BS — Breach Swap ────────────────────────────────────────────────────────
// Mace hit → swap back to sword
void runBS(uint16_t maceVk, uint16_t swordVk, int delay) {
    RunGuard rg("bs");
    int d = std::max(0, delay);

    preciseSleep(30);
    focusMc();
    keyPress(maceVk, KEY_HOLD_MS);
    mouseClick(false, CLICK_HOLD_MS); WAIT(d);
    keyPress(swordVk, KEY_HOLD_MS);
}

// ── LS — Lunge Swap ─────────────────────────────────────────────────────────
// Sword → spear+lclick (concurrent) → sword → sword (double-tap)
void runLS(uint16_t swordVk, uint16_t spearVk) {
    RunGuard rg("ls");

    focusMc();
    preciseSleep(2);
    keyPress(swordVk, SLOT_HOLD_MS);
    slotLClick(spearVk, SLOT_HOLD_MS); CHECK;
    preciseSleep(8);                   CHECK;
    keyPress(swordVk, SLOT_HOLD_MS);
    preciseSleep(4);
    keyPress(swordVk, SLOT_HOLD_MS);
}

// ── IC — Insta Cart ─────────────────────────────────────────────────────────
// Place rail → draw bow → release → place cart
void runIC(uint16_t railVk, uint16_t bowVk, uint16_t cartVk, int bowHoldMs, int delay) {
    RunGuard rg("ic");
    int d = std::max(0, delay);
    int holdMs = std::max(50, bowHoldMs);

    focusMc();
    keyPress(railVk, KEY_HOLD_MS);   WAIT(d);
    mouseClick(true, CLICK_HOLD_MS); WAIT(d); // place rail

    keyPress(bowVk, KEY_HOLD_MS);    WAIT(d);
    CHECK;
    mouseDown(true);  // draw bow
    preciseSleep(holdMs);
    mouseUp(true);    // release bow
    WAIT(d);

    keyPress(cartVk, KEY_HOLD_MS);
    mouseClick(true, CLICK_HOLD_MS); // place cart
}

// ── XB — Crossbow Cart ──────────────────────────────────────────────────────
// Rail → rclick → cart → rclick → flint&steel → rclick → crossbow → rclick
void runXB(uint16_t railVk, uint16_t cartVk, uint16_t fnsVk, uint16_t crossbowVk, int delay) {
    RunGuard rg("xb");
    int d = std::max(0, delay);

    focusMc();
    keyPress(railVk, KEY_HOLD_MS);      WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);    WAIT(d);
    keyPress(cartVk, KEY_HOLD_MS);      WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);    WAIT(d);
    keyPress(fnsVk, KEY_HOLD_MS);       WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);    WAIT(d);
    keyPress(crossbowVk, KEY_HOLD_MS);  WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);
}

// ── DR — Drain ──────────────────────────────────────────────────────────────
void runDR(uint16_t bucketVk, int delay) {
    RunGuard rg("dr");
    int d = std::max(0, delay);

    focusMc();
    keyPress(bucketVk, KEY_HOLD_MS); WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);
}

// ── LW — Lava Web ──────────────────────────────────────────────────────────
// Place lava → pick up → place cobweb
void runLW(uint16_t lavaVk, uint16_t cobwebVk, int delay) {
    RunGuard rg("lw");
    int d = std::max(0, delay);

    focusMc();
    keyPress(lavaVk, KEY_HOLD_MS);      WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);    WAIT(d); // place lava
    mouseClick(true, CLICK_HOLD_MS);    WAIT(d); // pick lava back up
    keyPress(cobwebVk, KEY_HOLD_MS);    WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);             // place cobweb
}

// ── LA — Lava ───────────────────────────────────────────────────────────────
void runLA(uint16_t lavaVk, int delay) {
    RunGuard rg("la");
    int d = std::max(0, delay);

    focusMc();
    keyPress(lavaVk, KEY_HOLD_MS); WAIT(d);
    mouseClick(true, CLICK_HOLD_MS);
}

// ── FXP — Fast XP (hold-to-run) ────────────────────────────────────────────

static std::atomic<bool> g_fxpActive{false};
static std::thread g_fxpThread;

void startFXP(int delay) {
    if (g_fxpActive.load()) return;
    g_fxpActive.store(true);

    int d = std::max(1, delay);
    g_fxpThread = std::thread([d]() {
        while (g_fxpActive.load() && !g_cancel.load()) {
            mouseClick(true, CLICK_HOLD_MS);
            preciseSleep(d);
        }
        g_fxpActive.store(false);
    });
    g_fxpThread.detach();
}

void stopFXP() {
    g_fxpActive.store(false);
}

bool isFXPActive() { return g_fxpActive.load(); }

// ── AC — Auto Crystal (hold-to-run) ────────────────────────────────────────

static std::atomic<bool> g_acActive{false};
static std::thread g_acThread;

void startAC(uint16_t crystalVk, int delay, bool rmbMode) {
    if (g_acActive.load()) return;
    g_acActive.store(true);

    int d = std::max(0, delay);
    uint16_t slotVk = crystalVk ? crystalVk : keyToVK("5");

    g_acThread = std::thread([slotVk, d, rmbMode]() {
        focusMc();
        preciseSleep(2);
        keyPress(slotVk, SLOT_HOLD_MS); // switch to crystal slot once

        if (rmbMode) {
            // RMB mode: physical RMB already placed first crystal
            preciseSleep(d);
            if (!g_acActive.load()) return;
            mouseClick(false, CLICK_HOLD_MS); // hit crystal
            preciseSleep(d);
        }

        // Main loop: place (rClick) → wait → hit (lClick) → wait
        while (g_acActive.load() && !g_cancel.load()) {
            mouseClick(true, CLICK_HOLD_MS);  // place
            preciseSleep(d);
            if (!g_acActive.load() || g_cancel.load()) break;
            mouseClick(false, CLICK_HOLD_MS); // hit
            preciseSleep(d);
        }
        g_acActive.store(false);
    });
    g_acThread.detach();
}

void stopAC() {
    g_acActive.store(false);
}

bool isACActive() { return g_acActive.load(); }

// ── Stop All ────────────────────────────────────────────────────────────────

void stopAll() {
    g_cancel.store(true);
    stopFXP();
    stopAC();
    // Brief pause to let running macros exit their loops
    preciseSleep(5);
    g_cancel.store(false);
}

} // namespace macros
