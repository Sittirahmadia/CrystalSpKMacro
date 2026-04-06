// ─────────────────────────────────────────────────────────────────────────────
//  main.cpp — CrystalSpKMacro native application entry point
//
//  Single .exe, no Electron/Node.js, no runtime dependencies.
//  ~5-10MB vs ~200MB Electron build.
//
//  Architecture:
//    main.cpp    → Win32 message loop, glue
//    gui.cpp     → Dark-themed owner-drawn window
//    tray.cpp    → System tray icon + context menu
//    engine.cpp  → WH_KEYBOARD_LL / WH_MOUSE_LL hooks → dispatch macros
//    macros.cpp  → All 21 macro implementations (native SendInput)
//    input.cpp   → Low-level SendInput primitives + high-res timing
//    config.cpp  → JSON config load/save (nlohmann/json)
//    optimizer.cpp → Windows registry optimizations
// ─────────────────────────────────────────────────────────────────────────────

#include "config.h"
#include "engine.h"
#include "gui.h"
#include "tray.h"
#include "input.h"
#include "macros.h"
#include "optimizer.h"
#include "resource.h"

#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

// ── Globals ─────────────────────────────────────────────────────────────────

static Config g_config;
static std::string g_configPath;
static UINT_PTR g_focusTimer = 0;
static UINT_PTR g_priorityTimer = 0;
static UINT_PTR g_autoSaveTimer = 0;
static bool g_configDirty = false;

// ── Auto-save ───────────────────────────────────────────────────────────────

static void markDirty() {
    g_configDirty = true;
}

static void saveIfDirty() {
    if (g_configDirty) {
        config::save(g_configPath, g_config);
        g_configDirty = false;
    }
}

// ── Callbacks ───────────────────────────────────────────────────────────────

static void onConfigChanged(const Config& cfg) {
    g_config = cfg;
    engine::loadConfig(cfg);
    markDirty();
}

static void onStopAll() {
    engine::stopAll();
}

static void onFocusLockChanged(bool enabled) {
    engine::setFocusLock(enabled);
}

static void onApplyOpt(const std::string& key) {
    auto r = optimizer::applyOpt(key, g_config.settings.mcExePath);
    if (r.ok) {
        g_config.settings.appliedOpts = optimizer::getApplied();
        markDirty();
    }
}

static void onRevertOpt(const std::string& key) {
    auto r = optimizer::revertOpt(key, g_config.settings.mcExePath);
    if (r.ok) {
        g_config.settings.appliedOpts = optimizer::getApplied();
        markDirty();
    }
}

static void onExit() {
    saveIfDirty();
    optimizer::cleanup();
    engine::shutdown();
    cleanupTiming();
    tray::cleanup();
    PostQuitMessage(0);
}

// ── Timer callbacks ─────────────────────────────────────────────────────────

#ifdef _WIN32
static void CALLBACK focusTimerProc(HWND, UINT, UINT_PTR, DWORD) {
    engine::pollMcFocus();
    gui::setMcFocused(isMcFocused());
}

static void CALLBACK priorityTimerProc(HWND, UINT, UINT_PTR, DWORD) {
    // Re-apply MC priority if the optimization is active
    auto applied = optimizer::getApplied();
    for (auto& k : applied) {
        if (k == "priority") {
            optimizer::applyOpt("priority");
            break;
        }
    }
}

static void CALLBACK autoSaveTimerProc(HWND, UINT, UINT_PTR, DWORD) {
    saveIfDirty();
}
#endif

// ── Entry point ─────────────────────────────────────────────────────────────

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    // Prevent multiple instances
    HANDLE mutex = CreateMutexW(nullptr, TRUE, L"CrystalSpKMacro_SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Find and activate existing window
        HWND existing = FindWindowW(L"CrystalSpKMacroWnd", nullptr);
        if (existing) {
            ShowWindow(existing, SW_SHOW);
            SetForegroundWindow(existing);
        }
        return 0;
    }

    // Initialize timing (sets timer resolution to 1ms)
    initTiming();

    // Load config
    g_configPath = config::getDefaultPath();
    g_config = config::load(g_configPath);

    // Initialize engine (sets up keyboard/mouse hooks)
    engine::init();
    engine::loadConfig(g_config);
    engine::setFocusLock(g_config.settings.focusLock);

    // Restore previously applied optimizations
    if (!g_config.settings.appliedOpts.empty()) {
        optimizer::restoreApplied(g_config.settings.appliedOpts, g_config.settings.mcExePath);
    }

    // Create GUI
    gui::Callbacks guiCbs;
    guiCbs.onConfigChanged    = onConfigChanged;
    guiCbs.onStopAll          = onStopAll;
    guiCbs.onFocusLockChanged = onFocusLockChanged;
    guiCbs.onApplyOpt         = onApplyOpt;
    guiCbs.onRevertOpt        = onRevertOpt;
    guiCbs.onExit             = onExit;

    HWND hwnd = gui::create(hInstance, g_config, guiCbs);

    // Create system tray
    tray::TrayCallbacks trayCbs;
    trayCbs.onShow    = []() { gui::show(); };
    trayCbs.onHide    = []() { gui::hide(); };
    trayCbs.onStopAll = onStopAll;
    trayCbs.onExit    = onExit;
    tray::init(hwnd, hInstance, trayCbs);

    // Set up timers
    g_focusTimer    = SetTimer(hwnd, 1, 500, focusTimerProc);     // MC focus poll every 500ms
    g_priorityTimer = SetTimer(hwnd, 2, 30000, priorityTimerProc); // Priority re-apply every 30s
    g_autoSaveTimer = SetTimer(hwnd, 3, 5000, autoSaveTimerProc); // Auto-save every 5s

    // ── Message loop ────────────────────────────────────────────────────────
    // WH_KEYBOARD_LL and WH_MOUSE_LL hooks require a message loop to
    // dispatch their callbacks.  This is the standard Win32 message pump.
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        // Forward tray messages
        if (msg.message == WM_TRAYICON) {
            tray::handleMessage(hwnd, msg.message, msg.wParam, msg.lParam);
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Cleanup
    KillTimer(hwnd, g_focusTimer);
    KillTimer(hwnd, g_priorityTimer);
    KillTimer(hwnd, g_autoSaveTimer);
    saveIfDirty();
    optimizer::cleanup();
    engine::shutdown();
    cleanupTiming();
    tray::cleanup();

    if (mutex) {
        ReleaseMutex(mutex);
        CloseHandle(mutex);
    }

    return 0;
}
#else
int main() {
    std::cout << "CrystalSpKMacro native — Windows only.\n";
    return 1;
}
#endif
