// ─────────────────────────────────────────────────────────────────────────────
//  input.cpp — Win32 SendInput primitives + high-res timing
// ─────────────────────────────────────────────────────────────────────────────

#include "input.h"
#include <unordered_map>
#include <chrono>
#include <thread>
#include <algorithm>

#ifdef _WIN32
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winmm.lib")

// ── Timing ──────────────────────────────────────────────────────────────────

static bool g_timerInit = false;

void initTiming() {
    if (!g_timerInit) {
        timeBeginPeriod(1);
        g_timerInit = true;
    }
}

void cleanupTiming() {
    if (g_timerInit) {
        timeEndPeriod(1);
        g_timerInit = false;
    }
}

void preciseSleep(int ms) {
    if (ms <= 0) return;
    auto start = std::chrono::high_resolution_clock::now();
    if (ms > 3) {
        ::Sleep(static_cast<DWORD>(ms - 2));
    }
    // Spin-wait the final stretch for sub-ms accuracy
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
        if (elapsed >= static_cast<long long>(ms) * 1000) break;
    }
}

// ── Key mapping ─────────────────────────────────────────────────────────────

uint16_t keyToVK(const std::string& key) {
    if (key.empty() || key == "None" || key == "none") return 0;

    // Single digit: '0'-'9'
    if (key.size() == 1 && key[0] >= '0' && key[0] <= '9')
        return static_cast<uint16_t>(key[0]);

    // Single letter: 'a'-'z' or 'A'-'Z'
    if (key.size() == 1) {
        char c = key[0];
        if (c >= 'a' && c <= 'z') return static_cast<uint16_t>(c - 'a' + 'A');
        if (c >= 'A' && c <= 'Z') return static_cast<uint16_t>(c);
    }

    static const std::unordered_map<std::string, uint16_t> named = {
        {"f1", VK_F1}, {"f2", VK_F2}, {"f3", VK_F3}, {"f4", VK_F4},
        {"f5", VK_F5}, {"f6", VK_F6}, {"f7", VK_F7}, {"f8", VK_F8},
        {"f9", VK_F9}, {"f10", VK_F10}, {"f11", VK_F11}, {"f12", VK_F12},
        {"space", VK_SPACE}, {"enter", VK_RETURN}, {"return", VK_RETURN},
        {"escape", VK_ESCAPE}, {"esc", VK_ESCAPE}, {"tab", VK_TAB},
        {"backspace", VK_BACK}, {"delete", VK_DELETE}, {"insert", VK_INSERT},
        {"home", VK_HOME}, {"end", VK_END},
        {"pageup", VK_PRIOR}, {"pagedown", VK_NEXT},
        {"up", VK_UP}, {"down", VK_DOWN}, {"left", VK_LEFT}, {"right", VK_RIGHT},
        {"shift", VK_SHIFT}, {"lshift", VK_LSHIFT}, {"rshift", VK_RSHIFT},
        {"ctrl", VK_CONTROL}, {"lctrl", VK_LCONTROL}, {"rctrl", VK_RCONTROL},
        {"alt", VK_MENU}, {"lalt", VK_LMENU}, {"ralt", VK_RMENU},
        {"capslock", VK_CAPITAL}, {"numlock", VK_NUMLOCK},
        {"mouse1", VK_LBUTTON}, {"mouse2", VK_RBUTTON},
        {"mouse3", VK_MBUTTON}, {"mouse4", VK_XBUTTON1}, {"mouse5", VK_XBUTTON2},
        {"num0", VK_NUMPAD0}, {"num1", VK_NUMPAD1}, {"num2", VK_NUMPAD2},
        {"num3", VK_NUMPAD3}, {"num4", VK_NUMPAD4}, {"num5", VK_NUMPAD5},
        {"num6", VK_NUMPAD6}, {"num7", VK_NUMPAD7}, {"num8", VK_NUMPAD8},
        {"num9", VK_NUMPAD9},
        {"minus", VK_OEM_MINUS}, {"plus", VK_OEM_PLUS},
        {"comma", VK_OEM_COMMA}, {"period", VK_OEM_PERIOD},
    };

    // Convert to lowercase for lookup
    std::string lower = key;
    for (auto& c : lower) c = static_cast<char>(tolower(c));
    auto it = named.find(lower);
    return it != named.end() ? it->second : 0;
}

std::string vkToName(uint16_t vk) {
    if (vk >= '0' && vk <= '9') return std::string(1, static_cast<char>(vk));
    if (vk >= 'A' && vk <= 'Z') return std::string(1, static_cast<char>(vk - 'A' + 'a'));
    if (vk >= VK_F1 && vk <= VK_F12) return "F" + std::to_string(vk - VK_F1 + 1);

    static const std::unordered_map<uint16_t, std::string> names = {
        {VK_SPACE, "Space"}, {VK_RETURN, "Enter"}, {VK_ESCAPE, "Escape"},
        {VK_TAB, "Tab"}, {VK_BACK, "Backspace"}, {VK_DELETE, "Delete"},
        {VK_SHIFT, "Shift"}, {VK_CONTROL, "Ctrl"}, {VK_MENU, "Alt"},
        {VK_LBUTTON, "Mouse1"}, {VK_RBUTTON, "Mouse2"}, {VK_MBUTTON, "Mouse3"},
        {VK_XBUTTON1, "Mouse4"}, {VK_XBUTTON2, "Mouse5"},
        {VK_UP, "Up"}, {VK_DOWN, "Down"}, {VK_LEFT, "Left"}, {VK_RIGHT, "Right"},
        {VK_CAPITAL, "CapsLock"}, {VK_OEM_MINUS, "Minus"}, {VK_OEM_PLUS, "Plus"},
    };
    auto it = names.find(vk);
    return it != names.end() ? it->second : "VK" + std::to_string(vk);
}

// ── Keyboard SendInput ──────────────────────────────────────────────────────

void sendKeyDown(uint16_t vk) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.wScan = static_cast<WORD>(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC));
    input.ki.dwFlags = KEYEVENTF_SCANCODE;
    SendInput(1, &input, sizeof(INPUT));
}

void sendKeyUp(uint16_t vk) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.wScan = static_cast<WORD>(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC));
    input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void keyPress(uint16_t vk, int holdMs) {
    if (!vk) return;
    sendKeyDown(vk);
    preciseSleep(holdMs);
    sendKeyUp(vk);
}

// ── Mouse SendInput ─────────────────────────────────────────────────────────

void mouseDown(bool rightButton) {
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = rightButton ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));
}

void mouseUp(bool rightButton) {
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = rightButton ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

void mouseClick(bool rightButton, int holdMs) {
    mouseDown(rightButton);
    preciseSleep(holdMs);
    mouseUp(rightButton);
}

// ── Batched concurrent key + click ──────────────────────────────────────────

void slotClick(uint16_t vk, int holdMs) {
    if (!vk) return;
    INPUT inputs[2] = {};
    // Key down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vk;
    inputs[0].ki.wScan = static_cast<WORD>(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC));
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;
    // Mouse right down
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    SendInput(2, inputs, sizeof(INPUT));

    preciseSleep(holdMs);

    // Key up + mouse right up
    INPUT ups[2] = {};
    ups[0].type = INPUT_KEYBOARD;
    ups[0].ki.wVk = vk;
    ups[0].ki.wScan = static_cast<WORD>(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC));
    ups[0].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    ups[1].type = INPUT_MOUSE;
    ups[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(2, ups, sizeof(INPUT));
}

void slotLClick(uint16_t vk, int holdMs) {
    if (!vk) return;
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vk;
    inputs[0].ki.wScan = static_cast<WORD>(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC));
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(2, inputs, sizeof(INPUT));

    preciseSleep(holdMs);

    INPUT ups[2] = {};
    ups[0].type = INPUT_KEYBOARD;
    ups[0].ki.wVk = vk;
    ups[0].ki.wScan = static_cast<WORD>(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC));
    ups[0].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    ups[1].type = INPUT_MOUSE;
    ups[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, ups, sizeof(INPUT));
}

// ── Focus detection ─────────────────────────────────────────────────────────

HWND findMcWindow() {
    struct FindData { HWND result; };
    FindData data = { nullptr };

    EnumWindows([](HWND hwnd, LPARAM lp) -> BOOL {
        auto* d = reinterpret_cast<FindData*>(lp);
        wchar_t title[256] = {};
        GetWindowTextW(hwnd, title, 256);
        std::wstring t(title);
        // Minecraft window titles typically contain "Minecraft"
        if (t.find(L"Minecraft") != std::wstring::npos) {
            if (IsWindowVisible(hwnd)) {
                d->result = hwnd;
                return FALSE; // stop enumeration
            }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&data));

    return data.result;
}

bool isMcFocused() {
    HWND fg = GetForegroundWindow();
    if (!fg) return false;
    wchar_t title[256] = {};
    GetWindowTextW(fg, title, 256);
    return std::wstring(title).find(L"Minecraft") != std::wstring::npos;
}

void focusMc() {
    HWND mc = findMcWindow();
    if (mc && mc != GetForegroundWindow()) {
        SetForegroundWindow(mc);
        preciseSleep(2);
    }
}

#else
// ── Non-Windows stubs ───────────────────────────────────────────────────────
void initTiming() {}
void cleanupTiming() {}
void preciseSleep(int) {}
void sendKeyDown(uint16_t) {}
void sendKeyUp(uint16_t) {}
void keyPress(uint16_t, int) {}
void mouseDown(bool) {}
void mouseUp(bool) {}
void mouseClick(bool, int) {}
void slotClick(uint16_t, int) {}
void slotLClick(uint16_t, int) {}
bool isMcFocused() { return false; }
void focusMc() {}
#endif
