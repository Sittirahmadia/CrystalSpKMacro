#pragma once
// ─────────────────────────────────────────────────────────────────────────────
//  input.h — Low-level Win32 input primitives for CrystalSpKMacro
//
//  All functions use SendInput with hardware flags so Minecraft/GLFW
//  processes them regardless of focus state.
// ─────────────────────────────────────────────────────────────────────────────

#include <string>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

// ── Timing constants (ms) ───────────────────────────────────────────────────
constexpr int SLOT_HOLD_MS  = 12;   // key hold during slotClick
constexpr int CLICK_HOLD_MS = 8;    // standalone click hold
constexpr int KEY_HOLD_MS   = 15;   // regular key press hold

// ── Key mapping ─────────────────────────────────────────────────────────────
uint16_t keyToVK(const std::string& key);
std::string vkToName(uint16_t vk);

// ── High-resolution timing ──────────────────────────────────────────────────
void preciseSleep(int ms);
void initTiming();    // call once at startup (sets timer resolution)
void cleanupTiming(); // call on exit

// ── Keyboard ────────────────────────────────────────────────────────────────
void sendKeyDown(uint16_t vk);
void sendKeyUp(uint16_t vk);
void keyPress(uint16_t vk, int holdMs = KEY_HOLD_MS);

// ── Mouse ───────────────────────────────────────────────────────────────────
void mouseDown(bool rightButton);   // true=right, false=left
void mouseUp(bool rightButton);
void mouseClick(bool rightButton, int holdMs = CLICK_HOLD_MS);

// ── Batched (concurrent key + click in one SendInput call) ──────────────────
void slotClick(uint16_t vk, int holdMs = SLOT_HOLD_MS);   // key + right-click
void slotLClick(uint16_t vk, int holdMs = SLOT_HOLD_MS);  // key + left-click

// ── Focus ───────────────────────────────────────────────────────────────────
bool isMcFocused();
void focusMc();

#ifdef _WIN32
HWND findMcWindow();
#endif
