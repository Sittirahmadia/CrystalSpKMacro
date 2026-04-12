// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Low-Level Input Simulation (SendInput)    ║
// ║  Uses SendInput API for harder anti-cheat detection     ║
// ╚══════════════════════════════════════════════════════════╝

#include "input/input_simulator.h"
#include "utils/delay.h"
#include <cstring>

namespace noqwd {

// ─── Internal helper ────────────────────────────────────
void InputSimulator::send_mouse_input(DWORD flags, DWORD data) {
    INPUT input{};
    input.type           = INPUT_MOUSE;
    input.mi.dwFlags     = flags;
    input.mi.mouseData   = data;
    input.mi.dwExtraInfo = GetMessageExtraInfo();
    SendInput(1, &input, sizeof(INPUT));
}

void InputSimulator::send_key_input(WORD vk, bool is_key_up) {
    INPUT input{};
    input.type       = INPUT_KEYBOARD;
    input.ki.wVk     = vk;
    input.ki.wScan   = static_cast<WORD>(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC));
    input.ki.dwFlags = (is_key_up ? KEYEVENTF_KEYUP : 0) | KEYEVENTF_SCANCODE;
    input.ki.dwExtraInfo = GetMessageExtraInfo();
    SendInput(1, &input, sizeof(INPUT));
}

// ─── Mouse ──────────────────────────────────────────────
void InputSimulator::mouse_left_click() {
    mouse_left_down();
    Delay::precision_sleep_us(800);  // ~0.8ms hold for realism
    mouse_left_up();
}

void InputSimulator::mouse_left_down() {
    send_mouse_input(MOUSEEVENTF_LEFTDOWN);
}

void InputSimulator::mouse_left_up() {
    send_mouse_input(MOUSEEVENTF_LEFTUP);
}

void InputSimulator::mouse_right_click() {
    mouse_right_down();
    Delay::precision_sleep_us(800);
    mouse_right_up();
}

void InputSimulator::mouse_right_down() {
    send_mouse_input(MOUSEEVENTF_RIGHTDOWN);
}

void InputSimulator::mouse_right_up() {
    send_mouse_input(MOUSEEVENTF_RIGHTUP);
}

void InputSimulator::mouse_move_relative(int dx, int dy) {
    send_mouse_input(MOUSEEVENTF_MOVE, 0);
    INPUT input{};
    input.type       = INPUT_MOUSE;
    input.mi.dx      = dx;
    input.mi.dy      = dy;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &input, sizeof(INPUT));
}

// ─── Keyboard ───────────────────────────────────────────
void InputSimulator::key_press(WORD vk) {
    key_down(vk);
    Delay::precision_sleep_us(600);  // brief hold
    key_up(vk);
}

void InputSimulator::key_down(WORD vk) {
    send_key_input(vk, false);
}

void InputSimulator::key_up(WORD vk) {
    send_key_input(vk, true);
}

// ─── Hotbar Slot ────────────────────────────────────────
void InputSimulator::select_slot(int slot) {
    if (slot < 1 || slot > 9) return;
    WORD vk = static_cast<WORD>('0' + slot);  // '1' = 0x31 .. '9' = 0x39
    key_press(vk);
}

// ─── Combo Helpers ──────────────────────────────────────
void InputSimulator::swap_to_slot_and_click(int slot, bool right_click) {
    select_slot(slot);
    Delay::precision_sleep_us(400);
    if (right_click) {
        mouse_right_click();
    } else {
        mouse_left_click();
    }
}

void InputSimulator::sprint_reset() {
    // Release and re-press Ctrl (sprint key)
    key_up(VK_LCONTROL);
    Delay::precision_sleep_us(200);
    key_down(VK_LCONTROL);
}

void InputSimulator::w_tap(int release_ms, int press_ms) {
    // Release W
    key_up(0x57);  // 'W' key
    Delay::sleep(release_ms);
    // Re-press W
    key_down(0x57);
    Delay::sleep(press_ms);
}

} // namespace noqwd
