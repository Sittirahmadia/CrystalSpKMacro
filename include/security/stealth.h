#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Stealth / Anti-Detection Module           ║
// ║  Process hiding, window cloaking, memory hardening      ║
// ╚══════════════════════════════════════════════════════════╝

#include <windows.h>
#include <string>

namespace noqwd {

class Stealth {
public:
    // Initialise all stealth measures
    static void init();

    // ─── Process Hardening ──────────────────────────────
    // Randomise the window class name each launch
    static std::wstring generate_random_class_name();

    // Set process as critical (anti-kill) — use with caution
    static void set_debug_privilege();

    // Remove PE header from memory to hinder scanning
    static void erase_pe_header();

    // ─── Window Cloaking ────────────────────────────────
    // Use DWM to cloak the window from task switcher
    static void cloak_window(HWND hwnd);
    static void uncloak_window(HWND hwnd);

    // ─── Module Spoofing ────────────────────────────────
    // Rename loaded module in PEB to a benign name
    static void spoof_module_name(const std::wstring& fake_name);

    // ─── Misc ───────────────────────────────────────────
    // Raise timer resolution for precision sleep
    static void set_timer_resolution();
    static void restore_timer_resolution();

private:
    static bool initialized_;
};

} // namespace noqwd
