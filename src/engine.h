#pragma once
// ─────────────────────────────────────────────────────────────────────────────
//  engine.h — Keybind hook engine using WH_KEYBOARD_LL / WH_MOUSE_LL
// ─────────────────────────────────────────────────────────────────────────────

#include "config.h"
#include <functional>

namespace engine {

// Initialize the engine with macro config.  Must be called from main thread.
void init();
void shutdown();

// Load/reload macro bindings from config
void loadConfig(const Config& cfg);

// Focus lock control
void setFocusLock(bool enabled);
bool isFocusLocked();

// Chat pause (temporarily disable all macros)
void setChatPaused(bool paused);
bool isChatPaused();

// Stop all macros immediately
void stopAll();

// Call this periodically (e.g. from a timer) to check MC focus state
void pollMcFocus();

// Register a callback for macro status changes (for GUI updates)
using StatusCallback = std::function<void(const std::string& macroId, bool active)>;
void setStatusCallback(StatusCallback cb);

} // namespace engine
