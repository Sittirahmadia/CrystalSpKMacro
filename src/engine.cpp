// ─────────────────────────────────────────────────────────────────────────────
//  engine.cpp — Low-level keyboard/mouse hook engine for macro triggering
//
//  Uses WH_KEYBOARD_LL and WH_MOUSE_LL hooks to intercept keypresses
//  globally.  When a registered keybind fires, the corresponding macro
//  is dispatched to a worker thread so the hook callback returns fast.
// ─────────────────────────────────────────────────────────────────────────────

#include "engine.h"
#include "macros.h"
#include "input.h"
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

namespace engine {

// ── State ───────────────────────────────────────────────────────────────────

static Config g_config;
static std::mutex g_mtx;
static std::atomic<bool> g_focusLock{true};
static std::atomic<bool> g_chatPaused{false};
static std::atomic<bool> g_mcFocused{false};
static StatusCallback g_statusCb;

#ifdef _WIN32
static HHOOK g_kbHook = nullptr;
static HHOOK g_mouseHook = nullptr;
#endif

// Map: VK code → macro ID for one-shot macros
static std::unordered_map<uint16_t, std::string> g_keyBindings;

// Map: VK code → macro ID for hold-to-run macros (FXP, AC)
static std::unordered_map<uint16_t, std::string> g_holdBindings;

// Set of VK codes that are currently held (for hold-to-run)
static std::unordered_map<uint16_t, bool> g_heldKeys;

// ── Guard check ─────────────────────────────────────────────────────────────

static bool canFire() {
    if (g_chatPaused.load()) return false;
    if (g_focusLock.load() && !g_mcFocused.load()) return false;
    return true;
}

// ── Macro dispatch ──────────────────────────────────────────────────────────

static void dispatchMacro(const std::string& id) {
    if (!canFire()) return;

    std::lock_guard<std::mutex> lk(g_mtx);
    auto it = g_config.macros.find(id);
    if (it == g_config.macros.end()) return;
    const MacroConfig& mc = it->second;
    if (!mc.active) return;

    // Skip if already running (prevent double-fire from contact bounce)
    if (macros::isRunning(id)) return;

    // Dispatch to worker thread
    std::thread([id, mc]() {
        if (id == "sa") {
            macros::runSA(keyToVK(mc.slots["anchorKey"]),
                          keyToVK(mc.slots["glowstoneKey"]),
                          keyToVK(mc.slots["explodeKey"]),
                          mc.delay);
        }
        else if (id == "da") {
            macros::runDA(keyToVK(mc.slots["anchorKey"]),
                          keyToVK(mc.slots["glowstoneKey"]),
                          mc.delay);
        }
        else if (id == "ap") {
            macros::runAP(keyToVK(mc.slots["anchorKey"]),
                          keyToVK(mc.slots["glowstoneKey"]),
                          keyToVK(mc.slots["explodeKey"]),
                          keyToVK(mc.slots["pearlKey"]),
                          keyToVK(mc.slots["totemKey"]),
                          mc.delay);
        }
        else if (id == "hc") {
            macros::runHC(keyToVK(mc.slots["obsidianKey"]),
                          keyToVK(mc.slots["crystalKey"]),
                          mc.delay);
        }
        else if (id == "shc") {
            macros::runSHC(keyToVK(mc.slots["obsidianKey"]),
                           keyToVK(mc.slots["crystalKey"]),
                           mc.delay);
        }
        else if (id == "kp") {
            macros::runKP(keyToVK(mc.slots["pearlKey"]),
                          keyToVK(mc.slots["returnKey"]),
                          mc.delay);
        }
        else if (id == "idh") {
            macros::stopAll(); // stop anchors before opening inventory
            macros::runIDH(keyToVK(mc.slots["inventoryKey"]),
                           keyToVK(mc.slots["totemKey"]),
                           mc.delay);
        }
        else if (id == "oht") {
            macros::runOHT(keyToVK(mc.slots["totemKey"]),
                           keyToVK(mc.slots["swapKey"]),
                           mc.delay);
        }
        else if (id == "asb") {
            macros::runASB(keyToVK(mc.slots["axeKey"]),
                           keyToVK(mc.slots["swordKey"]),
                           mc.delay);
        }
        else if (id == "es") {
            macros::runES(keyToVK(mc.slots["elytraKey"]),
                          keyToVK(mc.slots["returnKey"]),
                          mc.delay);
        }
        else if (id == "pc") {
            macros::runPC(keyToVK(mc.slots["pearlKey"]),
                          keyToVK(mc.slots["windChargeKey"]),
                          mc.delay);
        }
        else if (id == "ss") {
            macros::runSS(keyToVK(mc.slots["axeKey"]),
                          keyToVK(mc.slots["maceKey"]),
                          mc.delay);
        }
        else if (id == "bs") {
            macros::runBS(keyToVK(mc.slots["maceKey"]),
                          keyToVK(mc.slots["swordKey"]),
                          mc.delay);
        }
        else if (id == "ls") {
            macros::runLS(keyToVK(mc.slots["swordKey"]),
                          keyToVK(mc.slots["spearKey"]));
        }
        else if (id == "ic") {
            int bowHold = 150;
            auto bh = mc.slots.find("bowHoldMs");
            if (bh != mc.slots.end()) {
                try { bowHold = std::stoi(bh->second); } catch (...) {}
            }
            macros::runIC(keyToVK(mc.slots["railKey"]),
                          keyToVK(mc.slots["bowKey"]),
                          keyToVK(mc.slots["cartKey"]),
                          bowHold, mc.delay);
        }
        else if (id == "xb") {
            macros::runXB(keyToVK(mc.slots["railKey"]),
                          keyToVK(mc.slots["cartKey"]),
                          keyToVK(mc.slots["fnsKey"]),
                          keyToVK(mc.slots["crossbowKey"]),
                          mc.delay);
        }
        else if (id == "dr") {
            macros::runDR(keyToVK(mc.slots["bucketKey"]), mc.delay);
        }
        else if (id == "lw") {
            macros::runLW(keyToVK(mc.slots["lavaKey"]),
                          keyToVK(mc.slots["cobwebKey"]),
                          mc.delay);
        }
        else if (id == "la") {
            macros::runLA(keyToVK(mc.slots["lavaKey"]), mc.delay);
        }
    }).detach();
}

// ── Hold-to-run dispatch ────────────────────────────────────────────────────

static void dispatchHoldDown(const std::string& id) {
    if (!canFire()) return;

    std::lock_guard<std::mutex> lk(g_mtx);
    auto it = g_config.macros.find(id);
    if (it == g_config.macros.end()) return;
    const MacroConfig& mc = it->second;
    if (!mc.active) return;

    if (id == "fxp") {
        macros::startFXP(mc.delay > 0 ? mc.delay : 35);
    }
    else if (id == "ac") {
        uint16_t crystalVk = keyToVK(mc.slots.count("crystalKey") ?
            mc.slots.at("crystalKey") : "5");
        bool rmbMode = (mc.keybind == "Mouse2");
        macros::startAC(crystalVk, mc.delay > 0 ? mc.delay : 25, rmbMode);
    }
}

static void dispatchHoldUp(const std::string& id) {
    if (id == "fxp") macros::stopFXP();
    else if (id == "ac") macros::stopAC();
}

// ── Hook callbacks ──────────────────────────────────────────────────────────

#ifdef _WIN32

// Check if the event was injected by SendInput (our own output)
static bool isInjected(DWORD flags) {
    return (flags & LLKHF_INJECTED) != 0;
}

static bool isMouseInjected(DWORD flags) {
    return (flags & LLMHF_INJECTED) != 0;
}

static LRESULT CALLBACK kbHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        auto* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        uint16_t vk = static_cast<uint16_t>(kb->vkCode);

        if (!isInjected(kb->flags)) {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                // One-shot macro
                auto it = g_keyBindings.find(vk);
                if (it != g_keyBindings.end()) {
                    dispatchMacro(it->second);
                }
                // Hold-to-run (keydown)
                auto hit = g_holdBindings.find(vk);
                if (hit != g_holdBindings.end()) {
                    if (!g_heldKeys[vk]) {
                        g_heldKeys[vk] = true;
                        dispatchHoldDown(hit->second);
                    }
                }
            }
            else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                // Hold-to-run (keyup)
                auto hit = g_holdBindings.find(vk);
                if (hit != g_holdBindings.end()) {
                    g_heldKeys[vk] = false;
                    dispatchHoldUp(hit->second);
                }
            }
        }
    }
    return CallNextHookEx(g_kbHook, nCode, wParam, lParam);
}

// Map mouse button names to WM_* message + XBUTTON id
struct MouseBtnInfo {
    UINT downMsg;
    UINT upMsg;
    WORD xbutton; // 0 for non-X buttons
};

static uint16_t mouseMessageToVK(WPARAM wParam, MSLLHOOKSTRUCT* ms) {
    switch (wParam) {
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: return VK_MBUTTON;
        case WM_XBUTTONDOWN: case WM_XBUTTONUP:
            if (HIWORD(ms->mouseData) == XBUTTON1) return VK_XBUTTON1;
            if (HIWORD(ms->mouseData) == XBUTTON2) return VK_XBUTTON2;
            break;
    }
    return 0;
}

static LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        auto* ms = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);

        if (!isMouseInjected(ms->flags)) {
            uint16_t vk = mouseMessageToVK(wParam, ms);
            bool isDown = (wParam == WM_MBUTTONDOWN || wParam == WM_XBUTTONDOWN);
            bool isUp   = (wParam == WM_MBUTTONUP || wParam == WM_XBUTTONUP);

            if (vk && isDown) {
                // One-shot
                auto it = g_keyBindings.find(vk);
                if (it != g_keyBindings.end()) {
                    dispatchMacro(it->second);
                }
                // Hold-to-run
                auto hit = g_holdBindings.find(vk);
                if (hit != g_holdBindings.end()) {
                    if (!g_heldKeys[vk]) {
                        g_heldKeys[vk] = true;
                        dispatchHoldDown(hit->second);
                    }
                }
            }
            else if (vk && isUp) {
                auto hit = g_holdBindings.find(vk);
                if (hit != g_holdBindings.end()) {
                    g_heldKeys[vk] = false;
                    dispatchHoldUp(hit->second);
                }
            }
        }
    }
    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

#endif // _WIN32

// ── Build binding maps from config ──────────────────────────────────────────

static void rebuildBindings() {
    g_keyBindings.clear();
    g_holdBindings.clear();
    g_heldKeys.clear();

    std::lock_guard<std::mutex> lk(g_mtx);
    for (auto& [id, mc] : g_config.macros) {
        if (!mc.active || mc.keybind.empty() || mc.keybind == "None") continue;

        uint16_t vk = keyToVK(mc.keybind);
        if (!vk) continue;

        // Hold-to-run macros
        if (id == "fxp" || id == "ac") {
            g_holdBindings[vk] = id;
        } else {
            // Skip if already bound (first macro wins)
            if (g_keyBindings.count(vk) == 0 && g_holdBindings.count(vk) == 0) {
                g_keyBindings[vk] = id;
            }
        }
    }
}

// ── Public API ──────────────────────────────────────────────────────────────

void init() {
#ifdef _WIN32
    g_kbHook = SetWindowsHookExW(WH_KEYBOARD_LL, kbHookProc, nullptr, 0);
    g_mouseHook = SetWindowsHookExW(WH_MOUSE_LL, mouseHookProc, nullptr, 0);
#endif
}

void shutdown() {
    macros::stopAll();
#ifdef _WIN32
    if (g_kbHook) { UnhookWindowsHookEx(g_kbHook); g_kbHook = nullptr; }
    if (g_mouseHook) { UnhookWindowsHookEx(g_mouseHook); g_mouseHook = nullptr; }
#endif
}

void loadConfig(const Config& cfg) {
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        g_config = cfg;
    }
    rebuildBindings();
}

void setFocusLock(bool enabled) {
    g_focusLock.store(enabled);
}

bool isFocusLocked() {
    return g_focusLock.load();
}

void setChatPaused(bool paused) {
    g_chatPaused.store(paused);
    if (paused) macros::stopAll();
}

bool isChatPaused() {
    return g_chatPaused.load();
}

void stopAll() {
    macros::stopAll();
}

void pollMcFocus() {
    g_mcFocused.store(isMcFocused());
}

void setStatusCallback(StatusCallback cb) {
    g_statusCb = std::move(cb);
}

} // namespace engine
