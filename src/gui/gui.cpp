// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Main GUI Controller                       ║
// ║  ImGui + DirectX 11 overlay with sidebar navigation     ║
// ║  All item switching via keybinds, triggers on side btns ║
// ╚══════════════════════════════════════════════════════════╝

#include "gui/gui.h"
#include "gui/theme.h"
#include "gui/widgets.h"
#include "config.h"
#include "macros/crystal.h"
#include "macros/mace.h"
#include "macros/sword.h"
#include "security/streamproof.h"
#include "security/stealth.h"
#include "utils/keybind.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>
#include <tchar.h>
#include <cstdio>

// Forward declare ImGui Win32 handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ─── Global settings instances ──────────────────────────
static noqwd::CrystalSettings g_crystal;
static noqwd::MaceSettings    g_mace;
static noqwd::SwordSettings   g_sword;

namespace noqwd {

// ═════════════════════════════════════════════════════════
//  Constructor / Destructor
// ═════════════════════════════════════════════════════════
GUI::GUI() {}
GUI::~GUI() { shutdown(); }

// ═════════════════════════════════════════════════════════
//  Initialisation
// ═════════════════════════════════════════════════════════
bool GUI::init() {
    auto& kb = KeybindManager::instance();

    // ── Macro Trigger Keybinds (default to mouse side buttons) ──
    kb.register_bind("hc_trigger",           VK_XBUTTON1);  // Mouse4
    kb.register_bind("sa_trigger",           VK_XBUTTON1);
    kb.register_bind("da_trigger",           VK_XBUTTON2);  // Mouse5
    kb.register_bind("ap_trigger",           VK_XBUTTON2);
    kb.register_bind("mace_trigger",         VK_XBUTTON1);
    kb.register_bind("wind_charge_trigger",  VK_XBUTTON2);
    kb.register_bind("wtap_trigger",         VK_XBUTTON1);
    kb.register_bind("sprint_reset_trigger", VK_XBUTTON2);

    // ── Item Switch Keybinds (user assigns their keyboard keys) ──
    // Crystal
    kb.register_bind("hc_obsidian_key");
    kb.register_bind("hc_crystal_key");
    kb.register_bind("sa_anchor_key");
    kb.register_bind("sa_glowstone_key");
    kb.register_bind("da_anchor_key");
    kb.register_bind("da_glowstone_key");
    kb.register_bind("ap_anchor_key");
    kb.register_bind("ap_glowstone_key");
    kb.register_bind("ap_pearl_key");

    // Mace
    kb.register_bind("mace_key");
    kb.register_bind("wind_charge_key");
    kb.register_bind("mace_combo_key");

    // Sword
    kb.register_bind("sword_key");

    if (!create_window()) return false;
    if (!create_device())  return false;
    setup_imgui();

    return true;
}

// ═════════════════════════════════════════════════════════
//  Window Creation
// ═════════════════════════════════════════════════════════
bool GUI::create_window() {
    std::wstring class_name = Stealth::generate_random_class_name();

    wc_ = {};
    wc_.cbSize        = sizeof(WNDCLASSEXW);
    wc_.style         = CS_CLASSDC;
    wc_.lpfnWndProc   = GUI::wnd_proc;
    wc_.hInstance      = GetModuleHandleW(nullptr);
    wc_.hCursor        = LoadCursor(nullptr, IDC_ARROW);

    static wchar_t persistent_name[64];
    wcsncpy_s(persistent_name, class_name.c_str(), 63);
    wc_.lpszClassName = persistent_name;

    RegisterClassExW(&wc_);

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    hwnd_ = CreateWindowExW(
        0, persistent_name,
        L"noqwdmacro",
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, wc_.hInstance, nullptr
    );

    if (!hwnd_) return false;

    SetWindowLongPtrW(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    ShowWindow(hwnd_, SW_SHOWDEFAULT);
    UpdateWindow(hwnd_);

    return true;
}

// ═════════════════════════════════════════════════════════
//  WndProc
// ═════════════════════════════════════════════════════════
LRESULT CALLBACK GUI::wnd_proc(HWND hwnd, UINT msg,
                                WPARAM wp, LPARAM lp) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (wp == SIZE_MINIMIZED) return 0;
            {
                auto* gui = reinterpret_cast<GUI*>(
                    GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                if (gui && gui->device_) {
                    gui->cleanup_render_target();
                    gui->swap_chain_->ResizeBuffers(
                        0, LOWORD(lp), HIWORD(lp),
                        DXGI_FORMAT_UNKNOWN, 0);
                    gui->create_render_target();
                }
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ═════════════════════════════════════════════════════════
//  DirectX 11 Device
// ═════════════════════════════════════════════════════════
bool GUI::create_device() {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount        = 2;
    sd.BufferDesc.Width   = 0;
    sd.BufferDesc.Height  = 0;
    sd.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow       = hwnd_;
    sd.SampleDesc.Count   = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed           = TRUE;
    sd.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL feature_level;
    const D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        levels, 2, D3D11_SDK_VERSION,
        &sd, &swap_chain_, &device_, &feature_level, &context_
    );

    if (FAILED(hr)) return false;

    create_render_target();
    return true;
}

void GUI::cleanup_device() {
    cleanup_render_target();
    if (swap_chain_) { swap_chain_->Release(); swap_chain_ = nullptr; }
    if (context_)    { context_->Release();    context_    = nullptr; }
    if (device_)     { device_->Release();     device_     = nullptr; }
}

void GUI::create_render_target() {
    ID3D11Texture2D* back_buffer = nullptr;
    swap_chain_->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    if (back_buffer) {
        device_->CreateRenderTargetView(back_buffer, nullptr, &rtv_);
        back_buffer->Release();
    }
}

void GUI::cleanup_render_target() {
    if (rtv_) { rtv_->Release(); rtv_ = nullptr; }
}

// ═════════════════════════════════════════════════════════
//  ImGui Setup
// ═════════════════════════════════════════════════════════
void GUI::setup_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    Theme::setup_fonts(15.0f);
    Theme::apply(ThemePreset::Dark);

    ImGui_ImplWin32_Init(hwnd_);
    ImGui_ImplDX11_Init(device_, context_);
}

// ═════════════════════════════════════════════════════════
//  Render Frame
// ═════════════════════════════════════════════════════════
bool GUI::render_frame() {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        if (msg.message == WM_QUIT)
            running_ = false;
    }

    if (!running_) return false;

    // Poll keybinds
    auto& kb = KeybindManager::instance();
    kb.update();

    // Sync keybind VK codes → macro settings
    // (so macros always have the latest keybind assignments)
    g_crystal.hc_obsidian_key  = kb.get("hc_obsidian_key").vk_code;
    g_crystal.hc_crystal_key   = kb.get("hc_crystal_key").vk_code;
    g_crystal.sa_anchor_key    = kb.get("sa_anchor_key").vk_code;
    g_crystal.sa_glowstone_key = kb.get("sa_glowstone_key").vk_code;
    g_crystal.da_anchor_key    = kb.get("da_anchor_key").vk_code;
    g_crystal.da_glowstone_key = kb.get("da_glowstone_key").vk_code;
    g_crystal.ap_anchor_key    = kb.get("ap_anchor_key").vk_code;
    g_crystal.ap_glowstone_key = kb.get("ap_glowstone_key").vk_code;
    g_crystal.ap_pearl_key     = kb.get("ap_pearl_key").vk_code;
    g_mace.mace_key            = kb.get("mace_key").vk_code;
    g_mace.wind_charge_key     = kb.get("wind_charge_key").vk_code;
    g_mace.mace_combo_key      = kb.get("mace_combo_key").vk_code;
    g_sword.sword_key          = kb.get("sword_key").vk_code;

    // Start ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    render_ui();

    ImGui::Render();
    const float clear[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context_->OMSetRenderTargets(1, &rtv_, nullptr);
    context_->ClearRenderTargetView(rtv_, clear);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    swap_chain_->Present(1, 0);

    return true;
}

// ═════════════════════════════════════════════════════════
//  Main UI Layout
// ═════════════════════════════════════════════════════════
void GUI::render_ui() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGui::Begin("##MainWindow", nullptr, flags);
    ImGui::PopStyleVar(2);

    float total_h = ImGui::GetContentRegionAvail().y;
    float sidebar_w = SIDEBAR_WIDTH;
    float status_h  = STATUSBAR_HEIGHT;

    // Sidebar (left)
    ImGui::BeginChild("##Sidebar", ImVec2(sidebar_w, total_h - status_h), true);
    render_sidebar();
    ImGui::EndChild();

    ImGui::SameLine();

    // Main panel (right)
    ImGui::BeginChild("##MainPanel",
                      ImVec2(0, total_h - status_h), true);
    render_main_panel();
    ImGui::EndChild();

    // Status bar (bottom)
    ImGui::BeginChild("##StatusBar", ImVec2(0, 0), true);
    render_status_bar();
    ImGui::EndChild();

    ImGui::End();
}

// ═════════════════════════════════════════════════════════
//  Sidebar
// ═════════════════════════════════════════════════════════
void GUI::render_sidebar() {
    ImGui::Spacing();
    ImGui::Spacing();

    ImVec4 accent = ImGui::GetStyle().Colors[ImGuiCol_CheckMark];
    ImGui::PushStyleColor(ImGuiCol_Text, accent);

    float text_w = ImGui::CalcTextSize("noqwdmacro").x;
    float avail  = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX((avail - text_w) * 0.5f);
    ImGui::Text("noqwdmacro");
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    struct MenuItem {
        const char*  label;
        MenuCategory cat;
    };

    MenuItem items[] = {
        { "  Crystal",  MenuCategory::Crystal  },
        { "  Mace",     MenuCategory::Mace     },
        { "  Sword",    MenuCategory::Sword    },
        { "  Other",    MenuCategory::Other    },
        { "  Themes",   MenuCategory::Themes   },
        { "  Settings", MenuCategory::Settings },
    };

    for (auto& item : items) {
        bool selected = (active_menu_ == item.cat);

        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4(accent.x * 0.3f, accent.y * 0.3f, accent.z * 0.3f, 0.5f));
        }

        if (ImGui::Button(item.label, ImVec2(ImGui::GetContentRegionAvail().x, 32))) {
            active_menu_ = item.cat;
        }

        if (selected) {
            ImGui::PopStyleColor();
        }
    }
}

// ═════════════════════════════════════════════════════════
//  Main Panel (dispatches to category)
// ═════════════════════════════════════════════════════════
void GUI::render_main_panel() {
    ImGui::Spacing();

    switch (active_menu_) {
        case MenuCategory::Crystal:  render_crystal_panel();  break;
        case MenuCategory::Mace:     render_mace_panel();     break;
        case MenuCategory::Sword:    render_sword_panel();    break;
        case MenuCategory::Other:    render_other_panel();    break;
        case MenuCategory::Themes:   render_themes_panel();   break;
        case MenuCategory::Settings: render_settings_panel(); break;
    }
}

// ═════════════════════════════════════════════════════════
//  Crystal Panel
// ═════════════════════════════════════════════════════════
void GUI::render_crystal_panel() {
    // ─── Hit Crystal ────────────────────────────────────
    widgets::SectionHeader("Hit Crystal (HC)");

    widgets::ToggleSwitch("Enable HC", &g_crystal.hc_enabled);
    ImGui::Spacing();

    widgets::DelaySlider("Place Delay##hc", &g_crystal.hc_place_delay);
    widgets::DelaySlider("Break Delay##hc", &g_crystal.hc_break_delay);

    widgets::KeybindButton("Obsidian Key##hc",  "hc_obsidian_key");
    widgets::KeybindButton("Crystal Key##hc",   "hc_crystal_key");

    widgets::LabeledSeparator("Trigger");
    widgets::KeybindButton("Trigger##hc", "hc_trigger");
    ImGui::SameLine(); widgets::HelpMarker("Key/button to activate this macro. Default: Mouse4 (side button)");

    // ─── Single Anchor ──────────────────────────────────
    widgets::SectionHeader("Single Anchor (SA)");

    widgets::ToggleSwitch("Enable SA", &g_crystal.sa_enabled);
    ImGui::Spacing();

    widgets::DelaySlider("Place Delay##sa",    &g_crystal.sa_place_delay);
    widgets::DelaySlider("Charge Delay##sa",   &g_crystal.sa_charge_delay);
    widgets::DelaySlider("Detonate Delay##sa", &g_crystal.sa_detonate_delay);

    widgets::KeybindButton("Anchor Key##sa",    "sa_anchor_key");
    widgets::KeybindButton("Glowstone Key##sa", "sa_glowstone_key");

    widgets::LabeledSeparator("Trigger");
    widgets::KeybindButton("Trigger##sa", "sa_trigger");
    ImGui::SameLine(); widgets::HelpMarker("Default: Mouse4 (side button)");

    // ─── Double Anchor ──────────────────────────────────
    widgets::SectionHeader("Double Anchor (DA)");

    widgets::ToggleSwitch("Enable DA", &g_crystal.da_enabled);
    ImGui::Spacing();

    widgets::DelaySlider("First Delay##da",   &g_crystal.da_first_delay);
    widgets::DelaySlider("Second Delay##da",  &g_crystal.da_second_delay);
    widgets::DelaySlider("Between Delay##da", &g_crystal.da_between_delay);
    ImGui::SameLine(); widgets::HelpMarker("Delay between the two anchor detonations (default 75ms)");

    widgets::KeybindButton("Anchor Key##da",    "da_anchor_key");
    widgets::KeybindButton("Glowstone Key##da", "da_glowstone_key");

    widgets::LabeledSeparator("Trigger");
    widgets::KeybindButton("Trigger##da", "da_trigger");
    ImGui::SameLine(); widgets::HelpMarker("Default: Mouse5 (side button)");

    // ─── Anchor Pearl ───────────────────────────────────
    widgets::SectionHeader("Anchor Pearl (AP)");

    widgets::ToggleSwitch("Enable AP", &g_crystal.ap_enabled);
    ImGui::Spacing();

    widgets::DelaySlider("Detonate Delay##ap", &g_crystal.ap_detonate_delay);
    widgets::DelaySlider("Pearl Delay##ap",    &g_crystal.ap_pearl_delay);

    widgets::KeybindButton("Anchor Key##ap",    "ap_anchor_key");
    widgets::KeybindButton("Glowstone Key##ap", "ap_glowstone_key");
    widgets::KeybindButton("Pearl Key##ap",     "ap_pearl_key");

    widgets::LabeledSeparator("Trigger");
    widgets::KeybindButton("Trigger##ap", "ap_trigger");
    ImGui::SameLine(); widgets::HelpMarker("Default: Mouse5 (side button)");
}

// ═════════════════════════════════════════════════════════
//  Mace Panel
// ═════════════════════════════════════════════════════════
void GUI::render_mace_panel() {
    // ─── Mace Attack ────────────────────────────────────
    widgets::SectionHeader("Mace Attack");

    widgets::ToggleSwitch("Enable Mace", &g_mace.mace_enabled);
    ImGui::Spacing();

    widgets::DelaySlider("Attack Delay##mace", &g_mace.mace_attack_delay);
    widgets::KeybindButton("Mace Key##mace",   "mace_key");

    widgets::LabeledSeparator("Trigger");
    widgets::KeybindButton("Trigger##mace", "mace_trigger");
    ImGui::SameLine(); widgets::HelpMarker("Default: Mouse4 (side button)");

    // ─── Wind Charge Combo ──────────────────────────────
    widgets::SectionHeader("Wind Charge Combo");

    widgets::ToggleSwitch("Enable Wind Charge", &g_mace.wind_enabled);
    ImGui::Spacing();

    widgets::DelaySlider("Throw Delay##wind",  &g_mace.wind_charge_delay);
    widgets::DelaySlider("Swap Delay##wind",   &g_mace.wind_swap_delay);
    ImGui::SameLine(); widgets::HelpMarker("Time between wind charge throw and mace swap");
    widgets::DelaySlider("Attack Delay##wind", &g_mace.wind_attack_delay);

    widgets::KeybindButton("Wind Charge Key##wind", "wind_charge_key");
    widgets::KeybindButton("Mace Key##wind",        "mace_combo_key");

    widgets::LabeledSeparator("Trigger");
    widgets::KeybindButton("Trigger##wind", "wind_charge_trigger");
    ImGui::SameLine(); widgets::HelpMarker("Default: Mouse5 (side button)");
}

// ═════════════════════════════════════════════════════════
//  Sword Panel
// ═════════════════════════════════════════════════════════
void GUI::render_sword_panel() {
    // ─── W-Tap ──────────────────────────────────────────
    widgets::SectionHeader("W-Tap");

    widgets::ToggleSwitch("Enable W-Tap", &g_sword.wtap_enabled);
    ImGui::Spacing();

    widgets::DelaySlider("Release Delay##wtap", &g_sword.wtap_release_delay);
    widgets::DelaySlider("Press Delay##wtap",   &g_sword.wtap_press_delay);

    bool auto_atk = g_sword.wtap_auto_attack;
    if (widgets::ToggleSwitch("Auto Attack##wtap", &auto_atk))
        g_sword.wtap_auto_attack = auto_atk;
    ImGui::SameLine(); widgets::HelpMarker("Automatically left-click when W-Tap triggers");

    widgets::KeybindButton("Sword Key##wtap", "sword_key");

    widgets::LabeledSeparator("Trigger");
    widgets::KeybindButton("Trigger##wtap", "wtap_trigger");
    ImGui::SameLine(); widgets::HelpMarker("Default: Mouse4 (side button)");

    // ─── Sprint Reset ───────────────────────────────────
    widgets::SectionHeader("Sprint Reset");

    widgets::ToggleSwitch("Enable Sprint Reset", &g_sword.sprint_reset_enabled);
    ImGui::Spacing();

    widgets::DelaySlider("Reset Delay##sprint", &g_sword.sprint_reset_delay);

    bool dbl_tap = g_sword.sprint_double_tap;
    if (widgets::ToggleSwitch("Double-Tap W##sprint", &dbl_tap))
        g_sword.sprint_double_tap = dbl_tap;
    ImGui::SameLine(); widgets::HelpMarker("Use double-tap W instead of Ctrl toggle");

    widgets::LabeledSeparator("Trigger");
    widgets::KeybindButton("Trigger##sprint", "sprint_reset_trigger");
    ImGui::SameLine(); widgets::HelpMarker("Default: Mouse5 (side button)");
}

// ═════════════════════════════════════════════════════════
//  Other Panel (Jitter, Streamproof, Info)
// ═════════════════════════════════════════════════════════
void GUI::render_other_panel() {
    widgets::SectionHeader("Randomised Delay (Jitter)");

    ImGui::TextWrapped(
        "All delays include a random jitter of +/- [min, max] milliseconds "
        "to mimic human input and avoid pattern detection.");
    ImGui::Spacing();

    widgets::DelaySlider("Jitter Min##global", &g_crystal.jitter_min, 0, 20);
    widgets::DelaySlider("Jitter Max##global", &g_crystal.jitter_max, 1, 30);

    // Sync jitter across all settings
    g_mace.jitter_min  = g_crystal.jitter_min;
    g_mace.jitter_max  = g_crystal.jitter_max;
    g_sword.jitter_min = g_crystal.jitter_min;
    g_sword.jitter_max = g_crystal.jitter_max;

    widgets::SectionHeader("Streamproof");

    if (widgets::ToggleSwitch("Enable Streamproof", &streamproof_on_)) {
        if (streamproof_on_)
            Streamproof::enable(hwnd_);
        else
            Streamproof::disable(hwnd_);
    }
    ImGui::SameLine(); widgets::HelpMarker(
        "Hides this window from OBS, Discord, and other screen capture software "
        "using SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE).");

    widgets::SectionHeader("Information");

    ImGui::Text("Application: %s", APP_NAME);
    ImGui::Text("Version:     %s", APP_VERSION);
    ImGui::Text("Author:      %s", APP_AUTHOR);
    ImGui::Spacing();
    ImGui::TextDisabled("Input Method: SendInput (low-level)");
    ImGui::TextDisabled("Timer:        NtSetTimerResolution (0.5ms)");
    ImGui::TextDisabled("Triggers:     Mouse Side Buttons (configurable)");
}

// ═════════════════════════════════════════════════════════
//  Themes Panel
// ═════════════════════════════════════════════════════════
void GUI::render_themes_panel() {
    widgets::SectionHeader("Theme Selection");

    const int count = static_cast<int>(ThemePreset::Count);

    for (int i = 0; i < count; ++i) {
        auto preset = static_cast<ThemePreset>(i);
        bool selected = (theme_index_ == i);

        ImVec4 accent = Theme::accent_color(preset);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float y_center = pos.y + ImGui::GetTextLineHeight() * 0.5f;
        ImGui::GetWindowDrawList()->AddCircleFilled(
            ImVec2(pos.x + 8, y_center), 6.0f,
            ImGui::ColorConvertFloat4ToU32(accent));
        ImGui::Dummy(ImVec2(22, 0));
        ImGui::SameLine();

        char label[64];
        snprintf(label, sizeof(label), "%s%s",
                 Theme::name(preset),
                 selected ? "  [Active]" : "");

        if (ImGui::Selectable(label, selected, 0, ImVec2(0, 24))) {
            theme_index_ = i;
            Theme::apply(preset);
        }
    }
}

// ═════════════════════════════════════════════════════════
//  Settings Panel
// ═════════════════════════════════════════════════════════
void GUI::render_settings_panel() {
    widgets::SectionHeader("Application Settings");

    ImGui::TextWrapped("Configure global application behaviour.");
    ImGui::Spacing();

    ImGui::Text("GUI Scale:");
    static float gui_scale = 1.0f;
    ImGui::PushItemWidth(180);
    if (ImGui::SliderFloat("##guiscale", &gui_scale, 0.8f, 1.5f, "%.1fx")) {
        ImGui::GetIO().FontGlobalScale = gui_scale;
    }
    ImGui::PopItemWidth();

    ImGui::Spacing();
    ImGui::Spacing();

    widgets::LabeledSeparator("All Keybinds");

    auto& kb = KeybindManager::instance();
    for (auto& [name, bind] : kb.all_binds()) {
        widgets::StatusIndicator(
            (name + ": " + bind.label).c_str(),
            bind.vk_code != 0
        );
    }

    ImGui::Spacing();
    ImGui::Spacing();

    if (widgets::AccentButton("Reset All Keybinds", ImVec2(200, 30))) {
        for (auto& [name, bind] : kb.all_binds()) {
            kb.set(name, 0);
        }
        // Re-set default triggers to side buttons
        kb.set("hc_trigger",           VK_XBUTTON1);
        kb.set("sa_trigger",           VK_XBUTTON1);
        kb.set("da_trigger",           VK_XBUTTON2);
        kb.set("ap_trigger",           VK_XBUTTON2);
        kb.set("mace_trigger",         VK_XBUTTON1);
        kb.set("wind_charge_trigger",  VK_XBUTTON2);
        kb.set("wtap_trigger",         VK_XBUTTON1);
        kb.set("sprint_reset_trigger", VK_XBUTTON2);
    }

    ImGui::Spacing();

    if (widgets::AccentButton("Reset All Delays", ImVec2(200, 30))) {
        g_crystal = CrystalSettings{};
        g_mace    = MaceSettings{};
        g_sword   = SwordSettings{};
    }
}

// ═════════════════════════════════════════════════════════
//  Status Bar
// ═════════════════════════════════════════════════════════
void GUI::render_status_bar() {
    ImGui::Spacing();

    widgets::StatusIndicator(status_text_.c_str(), true);

    ImGui::SameLine(ImGui::GetWindowWidth() - 120);
    ImGui::TextDisabled("%s %s", APP_NAME, APP_VERSION);
}

// ═════════════════════════════════════════════════════════
//  Shutdown
// ═════════════════════════════════════════════════════════
void GUI::shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanup_device();

    if (hwnd_) {
        DestroyWindow(hwnd_);
        UnregisterClassW(wc_.lpszClassName, wc_.hInstance);
        hwnd_ = nullptr;
    }
}

} // namespace noqwd
