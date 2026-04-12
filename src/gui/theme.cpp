// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Theme Engine                              ║
// ║  Elegant dark themes with accent colour variants        ║
// ╚══════════════════════════════════════════════════════════╝

#include "gui/theme.h"
#include <imgui.h>

namespace noqwd {

// ─── Common Style (shared across all themes) ────────────
void Theme::apply_common_style() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Rounding
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 4.0f;
    style.PopupRounding     = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 3.0f;
    style.TabRounding       = 4.0f;

    // Spacing
    style.WindowPadding     = ImVec2(12, 12);
    style.FramePadding      = ImVec2(8, 5);
    style.ItemSpacing       = ImVec2(8, 6);
    style.ItemInnerSpacing  = ImVec2(6, 4);
    style.ScrollbarSize     = 12.0f;
    style.GrabMinSize       = 8.0f;

    // Borders
    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.PopupBorderSize   = 1.0f;
    style.TabBorderSize     = 0.0f;

    // Alignment
    style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);
    style.SeparatorTextAlign = ImVec2(0.0f, 0.5f);
}

// ─── Font Setup ─────────────────────────────────────────
void Theme::setup_fonts(float base_size) {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    // Use default font with custom size
    ImFontConfig config;
    config.SizePixels     = base_size;
    config.OversampleH    = 2;
    config.OversampleV    = 1;
    config.PixelSnapH     = true;
    io.Fonts->AddFontDefault(&config);

    // Build atlas
    io.Fonts->Build();
}

// ─── Theme Names ────────────────────────────────────────
const char* Theme::name(ThemePreset preset) {
    switch (preset) {
        case ThemePreset::Dark:     return "Dark";
        case ThemePreset::Midnight: return "Midnight";
        case ThemePreset::Carbon:   return "Carbon";
        case ThemePreset::Emerald:  return "Emerald";
        case ThemePreset::Crimson:  return "Crimson";
        case ThemePreset::Ocean:    return "Ocean";
        default:                    return "Unknown";
    }
}

// ─── Accent Colours ─────────────────────────────────────
ImVec4 Theme::accent_color(ThemePreset preset) {
    switch (preset) {
        case ThemePreset::Dark:     return ImVec4(0.45f, 0.55f, 0.95f, 1.0f);
        case ThemePreset::Midnight: return ImVec4(0.30f, 0.40f, 0.90f, 1.0f);
        case ThemePreset::Carbon:   return ImVec4(0.65f, 0.65f, 0.70f, 1.0f);
        case ThemePreset::Emerald:  return ImVec4(0.20f, 0.80f, 0.50f, 1.0f);
        case ThemePreset::Crimson:  return ImVec4(0.90f, 0.25f, 0.30f, 1.0f);
        case ThemePreset::Ocean:    return ImVec4(0.20f, 0.75f, 0.85f, 1.0f);
        default:                    return ImVec4(0.45f, 0.55f, 0.95f, 1.0f);
    }
}

// ─── Apply dispatcher ───────────────────────────────────
void Theme::apply(ThemePreset preset) {
    apply_common_style();
    switch (preset) {
        case ThemePreset::Dark:     apply_dark();     break;
        case ThemePreset::Midnight: apply_midnight(); break;
        case ThemePreset::Carbon:   apply_carbon();   break;
        case ThemePreset::Emerald:  apply_emerald();  break;
        case ThemePreset::Crimson:  apply_crimson();  break;
        case ThemePreset::Ocean:    apply_ocean();    break;
        default:                    apply_dark();     break;
    }
}

// ═════════════════════════════════════════════════════════
//  Dark (Default) — Blue-grey accent
// ═════════════════════════════════════════════════════════
void Theme::apply_dark() {
    ImVec4* c = ImGui::GetStyle().Colors;

    ImVec4 bg       = ImVec4(0.09f, 0.09f, 0.11f, 1.0f);
    ImVec4 bg_child = ImVec4(0.11f, 0.11f, 0.14f, 1.0f);
    ImVec4 accent   = ImVec4(0.45f, 0.55f, 0.95f, 1.0f);
    ImVec4 accent_h = ImVec4(0.55f, 0.65f, 1.00f, 1.0f);
    ImVec4 accent_a = ImVec4(0.35f, 0.45f, 0.85f, 1.0f);
    ImVec4 text     = ImVec4(0.92f, 0.92f, 0.95f, 1.0f);
    ImVec4 text_dim = ImVec4(0.55f, 0.55f, 0.60f, 1.0f);
    ImVec4 border   = ImVec4(0.18f, 0.18f, 0.22f, 1.0f);

    c[ImGuiCol_WindowBg]             = bg;
    c[ImGuiCol_ChildBg]              = bg_child;
    c[ImGuiCol_PopupBg]              = ImVec4(0.10f, 0.10f, 0.13f, 0.96f);
    c[ImGuiCol_Border]               = border;
    c[ImGuiCol_BorderShadow]         = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_FrameBg]              = ImVec4(0.14f, 0.14f, 0.17f, 1.0f);
    c[ImGuiCol_FrameBgHovered]       = ImVec4(0.18f, 0.18f, 0.22f, 1.0f);
    c[ImGuiCol_FrameBgActive]        = ImVec4(0.20f, 0.20f, 0.25f, 1.0f);
    c[ImGuiCol_TitleBg]              = bg;
    c[ImGuiCol_TitleBgActive]        = ImVec4(0.10f, 0.10f, 0.13f, 1.0f);
    c[ImGuiCol_TitleBgCollapsed]     = bg;
    c[ImGuiCol_MenuBarBg]            = bg_child;
    c[ImGuiCol_ScrollbarBg]          = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
    c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.25f, 0.25f, 0.30f, 1.0f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.38f, 1.0f);
    c[ImGuiCol_ScrollbarGrabActive]  = accent_a;
    c[ImGuiCol_CheckMark]            = accent;
    c[ImGuiCol_SliderGrab]           = accent;
    c[ImGuiCol_SliderGrabActive]     = accent_h;
    c[ImGuiCol_Button]               = ImVec4(0.16f, 0.16f, 0.20f, 1.0f);
    c[ImGuiCol_ButtonHovered]        = ImVec4(0.22f, 0.22f, 0.28f, 1.0f);
    c[ImGuiCol_ButtonActive]         = accent_a;
    c[ImGuiCol_Header]               = ImVec4(0.16f, 0.16f, 0.20f, 1.0f);
    c[ImGuiCol_HeaderHovered]        = ImVec4(0.22f, 0.22f, 0.28f, 1.0f);
    c[ImGuiCol_HeaderActive]         = accent_a;
    c[ImGuiCol_Separator]            = border;
    c[ImGuiCol_SeparatorHovered]     = accent;
    c[ImGuiCol_SeparatorActive]      = accent_h;
    c[ImGuiCol_ResizeGrip]           = ImVec4(0.20f, 0.20f, 0.25f, 0.5f);
    c[ImGuiCol_ResizeGripHovered]    = accent;
    c[ImGuiCol_ResizeGripActive]     = accent_h;
    c[ImGuiCol_Tab]                  = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    c[ImGuiCol_TabHovered]           = accent;
    c[ImGuiCol_TabSelected]          = accent_a;
    c[ImGuiCol_Text]                 = text;
    c[ImGuiCol_TextDisabled]         = text_dim;
    c[ImGuiCol_PlotLines]            = accent;
    c[ImGuiCol_PlotHistogram]        = accent;
}

// ═════════════════════════════════════════════════════════
//  Midnight — Deep blue-black
// ═════════════════════════════════════════════════════════
void Theme::apply_midnight() {
    ImVec4* c = ImGui::GetStyle().Colors;

    ImVec4 bg       = ImVec4(0.05f, 0.05f, 0.10f, 1.0f);
    ImVec4 bg_child = ImVec4(0.07f, 0.07f, 0.13f, 1.0f);
    ImVec4 accent   = ImVec4(0.30f, 0.40f, 0.90f, 1.0f);
    ImVec4 accent_h = ImVec4(0.40f, 0.50f, 1.00f, 1.0f);
    ImVec4 accent_a = ImVec4(0.25f, 0.35f, 0.80f, 1.0f);
    ImVec4 border   = ImVec4(0.12f, 0.12f, 0.22f, 1.0f);

    c[ImGuiCol_WindowBg]             = bg;
    c[ImGuiCol_ChildBg]              = bg_child;
    c[ImGuiCol_PopupBg]              = ImVec4(0.06f, 0.06f, 0.12f, 0.96f);
    c[ImGuiCol_Border]               = border;
    c[ImGuiCol_FrameBg]              = ImVec4(0.10f, 0.10f, 0.18f, 1.0f);
    c[ImGuiCol_FrameBgHovered]       = ImVec4(0.14f, 0.14f, 0.24f, 1.0f);
    c[ImGuiCol_FrameBgActive]        = ImVec4(0.16f, 0.16f, 0.28f, 1.0f);
    c[ImGuiCol_TitleBg]              = bg;
    c[ImGuiCol_TitleBgActive]        = ImVec4(0.06f, 0.06f, 0.12f, 1.0f);
    c[ImGuiCol_ScrollbarBg]          = ImVec4(0.04f, 0.04f, 0.08f, 1.0f);
    c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.20f, 0.20f, 0.35f, 1.0f);
    c[ImGuiCol_CheckMark]            = accent;
    c[ImGuiCol_SliderGrab]           = accent;
    c[ImGuiCol_SliderGrabActive]     = accent_h;
    c[ImGuiCol_Button]               = ImVec4(0.12f, 0.12f, 0.22f, 1.0f);
    c[ImGuiCol_ButtonHovered]        = ImVec4(0.18f, 0.18f, 0.30f, 1.0f);
    c[ImGuiCol_ButtonActive]         = accent_a;
    c[ImGuiCol_Header]               = ImVec4(0.12f, 0.12f, 0.22f, 1.0f);
    c[ImGuiCol_HeaderHovered]        = ImVec4(0.18f, 0.18f, 0.30f, 1.0f);
    c[ImGuiCol_HeaderActive]         = accent_a;
    c[ImGuiCol_Separator]            = border;
    c[ImGuiCol_Tab]                  = ImVec4(0.08f, 0.08f, 0.16f, 1.0f);
    c[ImGuiCol_TabHovered]           = accent;
    c[ImGuiCol_TabSelected]          = accent_a;
    c[ImGuiCol_Text]                 = ImVec4(0.88f, 0.90f, 0.96f, 1.0f);
    c[ImGuiCol_TextDisabled]         = ImVec4(0.45f, 0.45f, 0.58f, 1.0f);
}

// ═════════════════════════════════════════════════════════
//  Carbon — Pure dark grey, minimal accent
// ═════════════════════════════════════════════════════════
void Theme::apply_carbon() {
    ImVec4* c = ImGui::GetStyle().Colors;

    ImVec4 bg       = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
    ImVec4 bg_child = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
    ImVec4 accent   = ImVec4(0.65f, 0.65f, 0.70f, 1.0f);
    ImVec4 accent_h = ImVec4(0.80f, 0.80f, 0.85f, 1.0f);
    ImVec4 accent_a = ImVec4(0.50f, 0.50f, 0.55f, 1.0f);
    ImVec4 border   = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);

    c[ImGuiCol_WindowBg]       = bg;
    c[ImGuiCol_ChildBg]        = bg_child;
    c[ImGuiCol_PopupBg]        = ImVec4(0.11f, 0.11f, 0.11f, 0.96f);
    c[ImGuiCol_Border]         = border;
    c[ImGuiCol_FrameBg]        = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
    c[ImGuiCol_FrameBgActive]  = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);
    c[ImGuiCol_TitleBg]        = bg;
    c[ImGuiCol_TitleBgActive]  = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    c[ImGuiCol_ScrollbarBg]    = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
    c[ImGuiCol_ScrollbarGrab]  = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);
    c[ImGuiCol_CheckMark]      = accent;
    c[ImGuiCol_SliderGrab]     = accent;
    c[ImGuiCol_SliderGrabActive] = accent_h;
    c[ImGuiCol_Button]         = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    c[ImGuiCol_ButtonHovered]  = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    c[ImGuiCol_ButtonActive]   = accent_a;
    c[ImGuiCol_Header]         = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    c[ImGuiCol_HeaderHovered]  = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    c[ImGuiCol_HeaderActive]   = accent_a;
    c[ImGuiCol_Separator]      = border;
    c[ImGuiCol_Tab]            = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
    c[ImGuiCol_TabHovered]     = accent;
    c[ImGuiCol_TabSelected]    = accent_a;
    c[ImGuiCol_Text]           = ImVec4(0.90f, 0.90f, 0.90f, 1.0f);
    c[ImGuiCol_TextDisabled]   = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);
}

// ═════════════════════════════════════════════════════════
//  Emerald — Dark with green accents
// ═════════════════════════════════════════════════════════
void Theme::apply_emerald() {
    ImVec4* c = ImGui::GetStyle().Colors;

    ImVec4 bg       = ImVec4(0.07f, 0.09f, 0.08f, 1.0f);
    ImVec4 bg_child = ImVec4(0.09f, 0.12f, 0.10f, 1.0f);
    ImVec4 accent   = ImVec4(0.20f, 0.80f, 0.50f, 1.0f);
    ImVec4 accent_h = ImVec4(0.30f, 0.90f, 0.60f, 1.0f);
    ImVec4 accent_a = ImVec4(0.15f, 0.65f, 0.40f, 1.0f);
    ImVec4 border   = ImVec4(0.14f, 0.20f, 0.16f, 1.0f);

    c[ImGuiCol_WindowBg]       = bg;
    c[ImGuiCol_ChildBg]        = bg_child;
    c[ImGuiCol_PopupBg]        = ImVec4(0.08f, 0.10f, 0.09f, 0.96f);
    c[ImGuiCol_Border]         = border;
    c[ImGuiCol_FrameBg]        = ImVec4(0.12f, 0.16f, 0.13f, 1.0f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.22f, 0.18f, 1.0f);
    c[ImGuiCol_FrameBgActive]  = ImVec4(0.18f, 0.26f, 0.20f, 1.0f);
    c[ImGuiCol_TitleBg]        = bg;
    c[ImGuiCol_TitleBgActive]  = ImVec4(0.08f, 0.10f, 0.09f, 1.0f);
    c[ImGuiCol_ScrollbarBg]    = ImVec4(0.06f, 0.08f, 0.07f, 1.0f);
    c[ImGuiCol_ScrollbarGrab]  = ImVec4(0.20f, 0.30f, 0.24f, 1.0f);
    c[ImGuiCol_CheckMark]      = accent;
    c[ImGuiCol_SliderGrab]     = accent;
    c[ImGuiCol_SliderGrabActive] = accent_h;
    c[ImGuiCol_Button]         = ImVec4(0.14f, 0.20f, 0.16f, 1.0f);
    c[ImGuiCol_ButtonHovered]  = ImVec4(0.18f, 0.28f, 0.22f, 1.0f);
    c[ImGuiCol_ButtonActive]   = accent_a;
    c[ImGuiCol_Header]         = ImVec4(0.14f, 0.20f, 0.16f, 1.0f);
    c[ImGuiCol_HeaderHovered]  = ImVec4(0.18f, 0.28f, 0.22f, 1.0f);
    c[ImGuiCol_HeaderActive]   = accent_a;
    c[ImGuiCol_Separator]      = border;
    c[ImGuiCol_Tab]            = ImVec4(0.10f, 0.14f, 0.11f, 1.0f);
    c[ImGuiCol_TabHovered]     = accent;
    c[ImGuiCol_TabSelected]    = accent_a;
    c[ImGuiCol_Text]           = ImVec4(0.88f, 0.94f, 0.90f, 1.0f);
    c[ImGuiCol_TextDisabled]   = ImVec4(0.45f, 0.55f, 0.48f, 1.0f);
}

// ═════════════════════════════════════════════════════════
//  Crimson — Dark with red accents
// ═════════════════════════════════════════════════════════
void Theme::apply_crimson() {
    ImVec4* c = ImGui::GetStyle().Colors;

    ImVec4 bg       = ImVec4(0.09f, 0.07f, 0.07f, 1.0f);
    ImVec4 bg_child = ImVec4(0.12f, 0.09f, 0.09f, 1.0f);
    ImVec4 accent   = ImVec4(0.90f, 0.25f, 0.30f, 1.0f);
    ImVec4 accent_h = ImVec4(1.00f, 0.35f, 0.40f, 1.0f);
    ImVec4 accent_a = ImVec4(0.75f, 0.20f, 0.25f, 1.0f);
    ImVec4 border   = ImVec4(0.22f, 0.14f, 0.14f, 1.0f);

    c[ImGuiCol_WindowBg]       = bg;
    c[ImGuiCol_ChildBg]        = bg_child;
    c[ImGuiCol_PopupBg]        = ImVec4(0.10f, 0.08f, 0.08f, 0.96f);
    c[ImGuiCol_Border]         = border;
    c[ImGuiCol_FrameBg]        = ImVec4(0.16f, 0.12f, 0.12f, 1.0f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.15f, 0.15f, 1.0f);
    c[ImGuiCol_FrameBgActive]  = ImVec4(0.28f, 0.18f, 0.18f, 1.0f);
    c[ImGuiCol_TitleBg]        = bg;
    c[ImGuiCol_TitleBgActive]  = ImVec4(0.10f, 0.08f, 0.08f, 1.0f);
    c[ImGuiCol_ScrollbarBg]    = ImVec4(0.07f, 0.05f, 0.05f, 1.0f);
    c[ImGuiCol_ScrollbarGrab]  = ImVec4(0.30f, 0.18f, 0.18f, 1.0f);
    c[ImGuiCol_CheckMark]      = accent;
    c[ImGuiCol_SliderGrab]     = accent;
    c[ImGuiCol_SliderGrabActive] = accent_h;
    c[ImGuiCol_Button]         = ImVec4(0.20f, 0.13f, 0.13f, 1.0f);
    c[ImGuiCol_ButtonHovered]  = ImVec4(0.28f, 0.18f, 0.18f, 1.0f);
    c[ImGuiCol_ButtonActive]   = accent_a;
    c[ImGuiCol_Header]         = ImVec4(0.20f, 0.13f, 0.13f, 1.0f);
    c[ImGuiCol_HeaderHovered]  = ImVec4(0.28f, 0.18f, 0.18f, 1.0f);
    c[ImGuiCol_HeaderActive]   = accent_a;
    c[ImGuiCol_Separator]      = border;
    c[ImGuiCol_Tab]            = ImVec4(0.14f, 0.10f, 0.10f, 1.0f);
    c[ImGuiCol_TabHovered]     = accent;
    c[ImGuiCol_TabSelected]    = accent_a;
    c[ImGuiCol_Text]           = ImVec4(0.95f, 0.88f, 0.88f, 1.0f);
    c[ImGuiCol_TextDisabled]   = ImVec4(0.58f, 0.42f, 0.42f, 1.0f);
}

// ═════════════════════════════════════════════════════════
//  Ocean — Dark with teal / cyan accents
// ═════════════════════════════════════════════════════════
void Theme::apply_ocean() {
    ImVec4* c = ImGui::GetStyle().Colors;

    ImVec4 bg       = ImVec4(0.06f, 0.08f, 0.10f, 1.0f);
    ImVec4 bg_child = ImVec4(0.08f, 0.11f, 0.14f, 1.0f);
    ImVec4 accent   = ImVec4(0.20f, 0.75f, 0.85f, 1.0f);
    ImVec4 accent_h = ImVec4(0.30f, 0.85f, 0.95f, 1.0f);
    ImVec4 accent_a = ImVec4(0.15f, 0.60f, 0.70f, 1.0f);
    ImVec4 border   = ImVec4(0.12f, 0.18f, 0.22f, 1.0f);

    c[ImGuiCol_WindowBg]       = bg;
    c[ImGuiCol_ChildBg]        = bg_child;
    c[ImGuiCol_PopupBg]        = ImVec4(0.07f, 0.09f, 0.12f, 0.96f);
    c[ImGuiCol_Border]         = border;
    c[ImGuiCol_FrameBg]        = ImVec4(0.10f, 0.14f, 0.18f, 1.0f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.20f, 0.25f, 1.0f);
    c[ImGuiCol_FrameBgActive]  = ImVec4(0.16f, 0.24f, 0.30f, 1.0f);
    c[ImGuiCol_TitleBg]        = bg;
    c[ImGuiCol_TitleBgActive]  = ImVec4(0.07f, 0.09f, 0.12f, 1.0f);
    c[ImGuiCol_ScrollbarBg]    = ImVec4(0.05f, 0.07f, 0.09f, 1.0f);
    c[ImGuiCol_ScrollbarGrab]  = ImVec4(0.18f, 0.28f, 0.34f, 1.0f);
    c[ImGuiCol_CheckMark]      = accent;
    c[ImGuiCol_SliderGrab]     = accent;
    c[ImGuiCol_SliderGrabActive] = accent_h;
    c[ImGuiCol_Button]         = ImVec4(0.12f, 0.18f, 0.24f, 1.0f);
    c[ImGuiCol_ButtonHovered]  = ImVec4(0.16f, 0.26f, 0.32f, 1.0f);
    c[ImGuiCol_ButtonActive]   = accent_a;
    c[ImGuiCol_Header]         = ImVec4(0.12f, 0.18f, 0.24f, 1.0f);
    c[ImGuiCol_HeaderHovered]  = ImVec4(0.16f, 0.26f, 0.32f, 1.0f);
    c[ImGuiCol_HeaderActive]   = accent_a;
    c[ImGuiCol_Separator]      = border;
    c[ImGuiCol_Tab]            = ImVec4(0.09f, 0.13f, 0.17f, 1.0f);
    c[ImGuiCol_TabHovered]     = accent;
    c[ImGuiCol_TabSelected]    = accent_a;
    c[ImGuiCol_Text]           = ImVec4(0.88f, 0.94f, 0.96f, 1.0f);
    c[ImGuiCol_TextDisabled]   = ImVec4(0.42f, 0.52f, 0.58f, 1.0f);
}

} // namespace noqwd
