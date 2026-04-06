#pragma once
// ─────────────────────────────────────────────────────────────────────────────
//  macros.h — All macro declarations for CrystalSpKMacro native
// ─────────────────────────────────────────────────────────────────────────────

#include <string>
#include <atomic>
#include <cstdint>

namespace macros {

// Global cancel flag — set by stopAll(), checked by every macro step
extern std::atomic<bool> g_cancel;

// ── One-shot macros (fire once per trigger) ─────────────────────────────────
void runSA(uint16_t anchorVk, uint16_t glowstoneVk, uint16_t explodeVk, int delay);
void runDA(uint16_t anchorVk, uint16_t glowstoneVk, int delay);
void runAP(uint16_t anchorVk, uint16_t glowstoneVk, uint16_t explodeVk,
           uint16_t pearlVk, uint16_t totemVk, int delay);
void runHC(uint16_t obsidianVk, uint16_t crystalVk, int delay);
void runSHC(uint16_t obsidianVk, uint16_t crystalVk, int delay);
void runKP(uint16_t pearlVk, uint16_t returnVk, int delay);
void runIDH(uint16_t inventoryVk, uint16_t totemVk, int delay);
void runOHT(uint16_t totemVk, uint16_t swapVk, int delay);
void runASB(uint16_t axeVk, uint16_t swordVk, int delay);
void runES(uint16_t elytraVk, uint16_t returnVk, int delay);
void runPC(uint16_t pearlVk, uint16_t windChargeVk, int delay);
void runSS(uint16_t axeVk, uint16_t maceVk, int delay);
void runBS(uint16_t maceVk, uint16_t swordVk, int delay);
void runLS(uint16_t swordVk, uint16_t spearVk);
void runIC(uint16_t railVk, uint16_t bowVk, uint16_t cartVk, int bowHoldMs, int delay);
void runXB(uint16_t railVk, uint16_t cartVk, uint16_t fnsVk, uint16_t crossbowVk, int delay);
void runDR(uint16_t bucketVk, int delay);
void runLW(uint16_t lavaVk, uint16_t cobwebVk, int delay);
void runLA(uint16_t lavaVk, int delay);

// ── Hold-to-run macros (loop while held) ────────────────────────────────────
void startFXP(int delay);
void stopFXP();
bool isFXPActive();

void startAC(uint16_t crystalVk, int delay, bool rmbMode);
void stopAC();
bool isACActive();

// ── Control ─────────────────────────────────────────────────────────────────
void stopAll();

// Is any one-shot macro currently running?
bool isRunning(const std::string& id);

} // namespace macros
