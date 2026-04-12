#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Main GUI Controller                       ║
// ║  ImGui + DirectX 11 overlay with dark theme             ║
// ╚══════════════════════════════════════════════════════════╝

#include <windows.h>
#include <d3d11.h>
#include <string>

// Forward declarations
struct ImGuiIO;

namespace noqwd {

// Forward declare settings structs
struct CrystalSettings;
struct MaceSettings;
struct SwordSettings;

// Menu categories
enum class MenuCategory {
    Crystal,
    Mace,
    Sword,
    Other,
    Themes,
    Settings
};

class GUI {
public:
    GUI();
    ~GUI();

    // Initialise window, D3D11, and ImGui
    bool init();

    // Main render loop — returns false when window should close
    bool render_frame();

    // Cleanup
    void shutdown();

    // Get the HWND (for streamproof)
    HWND get_hwnd() const { return hwnd_; }

private:
    // ─── Window ─────────────────────────────────────────
    bool create_window();
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg,
                                      WPARAM wp, LPARAM lp);

    // ─── DirectX 11 ────────────────────────────────────
    bool create_device();
    void cleanup_device();
    void create_render_target();
    void cleanup_render_target();

    // ─── ImGui Rendering ────────────────────────────────
    void setup_imgui();
    void render_ui();

    // ─── UI Panels ──────────────────────────────────────
    void render_sidebar();
    void render_main_panel();
    void render_status_bar();

    // ─── Category Panels ────────────────────────────────
    void render_crystal_panel();
    void render_mace_panel();
    void render_sword_panel();
    void render_other_panel();
    void render_themes_panel();
    void render_settings_panel();

    // ─── State ──────────────────────────────────────────
    HWND                     hwnd_           = nullptr;
    WNDCLASSEXW              wc_             = {};
    ID3D11Device*            device_         = nullptr;
    ID3D11DeviceContext*     context_        = nullptr;
    IDXGISwapChain*          swap_chain_     = nullptr;
    ID3D11RenderTargetView*  rtv_            = nullptr;

    MenuCategory             active_menu_    = MenuCategory::Crystal;
    bool                     running_        = true;
    bool                     streamproof_on_ = false;

    // Theme index
    int                      theme_index_    = 0;

    // Connection status
    std::string              status_text_    = "Connected";
};

} // namespace noqwd
