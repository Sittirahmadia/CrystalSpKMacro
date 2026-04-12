// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Custom ImGui Widgets                      ║
// ╚══════════════════════════════════════════════════════════╝

#include "gui/widgets.h"
#include "utils/keybind.h"
#include "config.h"
#include <imgui_internal.h>
#include <cstdio>

namespace noqwd {
namespace widgets {

// ═════════════════════════════════════════════════════════
//  Toggle Switch — animated on/off switch
// ═════════════════════════════════════════════════════════
bool ToggleSwitch(const char* label, bool* value) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const float height = ImGui::GetFrameHeight();
    const float width  = height * 1.8f;
    const float radius = height * 0.40f;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);

    ImRect total_bb(pos, ImVec2(pos.x + width + style.ItemInnerSpacing.x + label_size.x,
                                 pos.y + height));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed) *value = !(*value);

    // Animate
    float t = *value ? 1.0f : 0.0f;

    // Colours
    ImU32 col_bg = *value
        ? ImGui::GetColorU32(ImVec4(0.20f, 0.70f, 0.45f, 1.0f))
        : ImGui::GetColorU32(ImVec4(0.30f, 0.30f, 0.35f, 1.0f));

    ImU32 col_knob = IM_COL32(240, 240, 240, 255);

    // Draw track
    ImVec2 track_min = pos;
    ImVec2 track_max = ImVec2(pos.x + width, pos.y + height);
    window->DrawList->AddRectFilled(track_min, track_max, col_bg, height * 0.5f);

    // Draw knob
    float knob_x = pos.x + radius + t * (width - 2.0f * radius);
    float knob_y = pos.y + height * 0.5f;
    window->DrawList->AddCircleFilled(ImVec2(knob_x, knob_y), radius - 1.5f, col_knob);

    // Draw label
    ImVec2 label_pos(pos.x + width + style.ItemInnerSpacing.x,
                     pos.y + style.FramePadding.y);
    ImGui::RenderText(label_pos, label);

    return pressed;
}

// ═════════════════════════════════════════════════════════
//  Keybind Button — click to capture, ESC to cancel
// ═════════════════════════════════════════════════════════
bool KeybindButton(const char* label, const std::string& bind_name) {
    auto& mgr = KeybindManager::instance();
    const auto& kb = mgr.get(bind_name);

    bool capturing = mgr.is_capturing();
    // Check if WE are the one being captured
    // (simple approach: label the button differently)

    char buf[128];
    if (capturing) {
        snprintf(buf, sizeof(buf), "[Press Key]##%s", label);
    } else {
        snprintf(buf, sizeof(buf), "[ %s ]##%s", kb.label.c_str(), label);
    }

    // Wider button
    ImGui::PushItemWidth(100);
    ImGui::Text("%s", label);
    ImGui::SameLine(180);

    bool changed = false;
    if (ImGui::Button(buf, ImVec2(100, 0))) {
        if (!capturing) {
            mgr.start_capture(bind_name);
        }
    }
    ImGui::PopItemWidth();

    return changed;
}

// ═════════════════════════════════════════════════════════
//  Delay Slider — integer ms value with display
// ═════════════════════════════════════════════════════════
bool DelaySlider(const char* label, int* value, int min_val, int max_val) {
    ImGui::PushItemWidth(180);
    char fmt[16];
    snprintf(fmt, sizeof(fmt), "%%d ms");
    bool changed = ImGui::SliderInt(label, value, min_val, max_val, fmt);
    ImGui::PopItemWidth();

    // Clamp
    if (*value < min_val) *value = min_val;
    if (*value > max_val) *value = max_val;

    return changed;
}

// ═════════════════════════════════════════════════════════
//  Slot Selector — dropdown 1-9
// ═════════════════════════════════════════════════════════
bool SlotSelector(const char* label, int* slot) {
    static const char* slots[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
    int idx = (*slot) - 1;
    if (idx < 0) idx = 0;
    if (idx > 8) idx = 8;

    ImGui::PushItemWidth(80);
    bool changed = ImGui::Combo(label, &idx, slots, 9);
    ImGui::PopItemWidth();

    if (changed) *slot = idx + 1;
    return changed;
}

// ═════════════════════════════════════════════════════════
//  Section Header
// ═════════════════════════════════════════════════════════
void SectionHeader(const char* text) {
    ImGui::Spacing();
    ImGui::Spacing();

    ImVec4 accent = ImGui::GetStyle().Colors[ImGuiCol_CheckMark];
    ImGui::PushStyleColor(ImGuiCol_Text, accent);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();

    ImGui::Separator();
    ImGui::Spacing();
}

// ═════════════════════════════════════════════════════════
//  Labeled Separator
// ═════════════════════════════════════════════════════════
void LabeledSeparator(const char* label) {
    ImGui::Spacing();
    ImGui::SeparatorText(label);
    ImGui::Spacing();
}

// ═════════════════════════════════════════════════════════
//  Help Marker (?) tooltip
// ═════════════════════════════════════════════════════════
void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// ═════════════════════════════════════════════════════════
//  Accent Button
// ═════════════════════════════════════════════════════════
bool AccentButton(const char* label, const ImVec2& size) {
    ImVec4 accent  = ImGui::GetStyle().Colors[ImGuiCol_CheckMark];
    ImVec4 hovered = ImVec4(accent.x * 1.15f, accent.y * 1.15f, accent.z * 1.15f, 1.0f);
    ImVec4 active  = ImVec4(accent.x * 0.85f, accent.y * 0.85f, accent.z * 0.85f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

    bool pressed = ImGui::Button(label, size);

    ImGui::PopStyleColor(4);
    return pressed;
}

// ═════════════════════════════════════════════════════════
//  Status Indicator — coloured dot + label
// ═════════════════════════════════════════════════════════
void StatusIndicator(const char* text, bool active) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float radius = 5.0f;
    float y_center = pos.y + ImGui::GetTextLineHeight() * 0.5f;

    ImU32 color = active
        ? IM_COL32(60, 200, 120, 255)   // green
        : IM_COL32(200, 60, 60, 255);   // red

    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(pos.x + radius, y_center), radius, color);

    ImGui::Dummy(ImVec2(radius * 2 + 6, 0));
    ImGui::SameLine();
    ImGui::TextUnformatted(text);
}

} // namespace widgets
} // namespace noqwd
