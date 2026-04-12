#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Low-Level Input Simulation (SendInput)    ║
// ╚══════════════════════════════════════════════════════════╝

#include <windows.h>
#include <cstdint>

namespace noqwd {

class InputSimulator {
public:
    // ─── Mouse ──────────────────────────────────────────
    static void mouse_left_click();
    static void mouse_left_down();
    static void mouse_left_up();

    static void mouse_right_click();
    static void mouse_right_down();
    static void mouse_right_up();

    static void mouse_move_relative(int dx, int dy);

    // ─── Keyboard ───────────────────────────────────────
    static void key_press(WORD vk);          // Down + Up
    static void key_down(WORD vk);
    static void key_up(WORD vk);

    // ─── Hotbar Slot (1-9 keys) ─────────────────────────
    static void select_slot(int slot);       // slot 1-9

    // ─── Combo Helpers ──────────────────────────────────
    static void swap_to_slot_and_click(int slot, bool right_click = false);
    static void sprint_reset();              // Release + re-press W
    static void w_tap(int release_ms, int press_ms);

private:
    static void send_mouse_input(DWORD flags, DWORD data = 0);
    static void send_key_input(WORD vk, bool key_up);
};

} // namespace noqwd
