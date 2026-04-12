#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Custom ImGui Widgets                      ║
// ║  Toggle switches, keybind buttons, styled sliders       ║
// ╚══════════════════════════════════════════════════════════╝

#include <imgui.h>
#include <string>

namespace noqwd {
namespace widgets {

    // ─── Toggle Switch ──────────────────────────────────
    // Returns true if the value changed
    bool ToggleSwitch(const char* label, bool* value);

    // ─── Keybind Button ─────────────────────────────────
    // Shows current keybind, click to capture new key
    // Returns true if the keybind changed
    bool KeybindButton(const char* label, const std::string& bind_name);

    // ─── Delay Slider ───────────────────────────────────
    // Integer slider with "ms" suffix, clamped to DELAY_MIN..DELAY_MAX
    bool DelaySlider(const char* label, int* value, int min_val = 1, int max_val = 500);

    // ─── Slot Selector ──────────────────────────────────
    // Dropdown for hotbar slot 1-9
    bool SlotSelector(const char* label, int* slot);

    // ─── Section Header ─────────────────────────────────
    void SectionHeader(const char* text);

    // ─── Separator with label ───────────────────────────
    void LabeledSeparator(const char* label);

    // ─── Tooltip ────────────────────────────────────────
    void HelpMarker(const char* desc);

    // ─── Styled Button ──────────────────────────────────
    bool AccentButton(const char* label, const ImVec2& size = ImVec2(0, 0));

    // ─── Status Indicator (coloured dot + text) ─────────
    void StatusIndicator(const char* text, bool active);

} // namespace widgets
} // namespace noqwd
