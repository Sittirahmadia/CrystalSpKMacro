#pragma once
// ─────────────────────────────────────────────────────────────────────────────
//  gui.h — Win32 dark-themed GUI for CrystalSpKMacro (v2 — Electron-matching)
//
//  Architecture:
//    - Custom owner-drawn window with double-buffered GDI
//    - Sidebar navigation (220px) with categorized pages
//    - 2-column macro card grid with expandable detail panels
//    - Detection bar showing MC focus state
//    - Active macro chip bar
//    - Scrollable content via WM_MOUSEWHEEL
//    - Keybind capture overlay
//    - Dark theme matching Electron CSS with subtle gradient effects
// ─────────────────────────────────────────────────────────────────────────────

#include "config.h"
#include <functional>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace gui {

// ── Pages ───────────────────────────────────────────────────────────────────
enum class Page {
    Crystal,
    Sword,
    Mace,
    Cart,
    UHC,
    Optimizer,
    Settings,
};

// ── Callbacks from GUI → main ───────────────────────────────────────────────
struct Callbacks {
    std::function<void(const Config&)>      onConfigChanged;
    std::function<void()>                   onStopAll;
    std::function<void(bool)>               onFocusLockChanged;
    std::function<void(const std::string&)> onApplyOpt;
    std::function<void(const std::string&)> onRevertOpt;
    std::function<void()>                   onExit;
};

// ── Lifecycle ───────────────────────────────────────────────────────────────
#ifdef _WIN32
HWND create(HINSTANCE hInstance, const Config& cfg, const Callbacks& cbs);
#endif
void destroy();

// ── State updates from external ─────────────────────────────────────────────
void setMacroRunning(const std::string& id, bool running);
void setMcFocused(bool focused);
void reloadConfig(const Config& cfg);
Config getCurrentConfig();

// ── Window control ──────────────────────────────────────────────────────────
void show();
void hide();
bool isVisible();
void invalidate(); // force repaint

} // namespace gui
