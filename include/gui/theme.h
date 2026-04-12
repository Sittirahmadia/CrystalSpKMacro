#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Theme Engine                              ║
// ║  Dark themes inspired by 198macro                       ║
// ╚══════════════════════════════════════════════════════════╝

struct ImVec4;

namespace noqwd {

enum class ThemePreset {
    Dark,         // Default dark theme
    Midnight,     // Deep blue-black
    Carbon,       // Pure dark grey
    Emerald,      // Dark with green accents
    Crimson,      // Dark with red accents
    Ocean,        // Dark with teal accents
    Count
};

class Theme {
public:
    // Apply a theme preset to ImGui
    static void apply(ThemePreset preset = ThemePreset::Dark);

    // Get theme name for UI display
    static const char* name(ThemePreset preset);

    // Get accent colour for the current theme
    static ImVec4 accent_color(ThemePreset preset);

    // Setup fonts (call once after ImGui context creation)
    static void setup_fonts(float base_size = 15.0f);

private:
    static void apply_dark();
    static void apply_midnight();
    static void apply_carbon();
    static void apply_emerald();
    static void apply_crimson();
    static void apply_ocean();

    // Shared rounding / spacing configuration
    static void apply_common_style();
};

} // namespace noqwd
