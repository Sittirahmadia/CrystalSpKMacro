// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Entry Point                               ║
// ║  Competitive gaming macro with ImGui + DirectX 11       ║
// ║  Version 1.4.0                                          ║
// ╚══════════════════════════════════════════════════════════╝

#include "gui/gui.h"
#include "security/stealth.h"
#include "security/streamproof.h"
#include "config.h"

#include <windows.h>

// ═════════════════════════════════════════════════════════
//  WinMain — Application entry point
// ═════════════════════════════════════════════════════════
int WINAPI WinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPSTR     lpCmdLine,
    _In_     int       nShowCmd)
{
    // Suppress unused parameter warnings
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;

    // ─── Initialise Stealth Systems ─────────────────────
    noqwd::Stealth::init();
    noqwd::Stealth::set_debug_privilege();
    noqwd::Stealth::spoof_module_name(L"explorer.exe");

    // ─── Create & Init GUI ──────────────────────────────
    noqwd::GUI gui;
    if (!gui.init()) {
        MessageBoxW(nullptr,
            L"Failed to initialise noqwdmacro.\n"
            L"Ensure DirectX 11 is supported on your system.",
            L"noqwdmacro - Error",
            MB_OK | MB_ICONERROR);
        return 1;
    }

    // ─── Apply Streamproof by default ───────────────────
    // (User can toggle this in the 'Other' panel)
    // noqwd::Streamproof::enable(gui.get_hwnd());

    // ─── Main Loop ──────────────────────────────────────
    while (gui.render_frame()) {
        // The render_frame() call handles:
        // - Win32 message pump
        // - Keybind polling
        // - ImGui rendering
        // - D3D11 presentation
        //
        // Macro execution would be triggered by keybind
        // callbacks here in a real deployment, e.g.:
        //
        //   if (KeybindManager::instance().is_just_pressed("hc"))
        //       hit_crystal_macro.execute();
    }

    // ─── Cleanup ────────────────────────────────────────
    noqwd::Stealth::restore_timer_resolution();

    return 0;
}
