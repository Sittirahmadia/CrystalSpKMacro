// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Keybind Manager                           ║
// ╚══════════════════════════════════════════════════════════╝

#include "utils/keybind.h"
#include <sstream>

namespace noqwd {

KeybindManager& KeybindManager::instance() {
    static KeybindManager mgr;
    return mgr;
}

void KeybindManager::register_bind(const std::string& name, int default_vk) {
    Keybind kb;
    kb.vk_code = default_vk;
    kb.label   = (default_vk == 0) ? "None" : vk_to_string(default_vk);
    binds_[name]      = kb;
    prev_state_[name] = false;
}

Keybind& KeybindManager::get(const std::string& name) {
    return binds_[name];
}

const Keybind& KeybindManager::get(const std::string& name) const {
    static Keybind empty;
    auto it = binds_.find(name);
    return (it != binds_.end()) ? it->second : empty;
}

void KeybindManager::set(const std::string& name, int vk_code) {
    binds_[name].vk_code = vk_code;
    binds_[name].label   = (vk_code == 0) ? "None" : vk_to_string(vk_code);
}

void KeybindManager::update() {
    // Handle capture mode — listen for any key press
    if (capturing_) {
        for (int vk = 1; vk < 256; ++vk) {
            // Skip modifier-only keys for capture trigger
            if (vk == VK_ESCAPE) {
                if (GetAsyncKeyState(vk) & 0x8000) {
                    cancel_capture();
                    return;
                }
            }
            if (vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_MBUTTON)
                continue;
            if (vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU)
                continue;

            if (GetAsyncKeyState(vk) & 0x8000) {
                set(capture_target_, vk);
                capturing_ = false;
                capture_target_.clear();
                return;
            }
        }
        return;  // Still capturing, skip normal update
    }

    // Normal polling
    for (auto& [name, kb] : binds_) {
        bool currently_down = false;
        if (kb.vk_code != 0) {
            currently_down = (GetAsyncKeyState(kb.vk_code) & 0x8000) != 0;
        }
        kb.active        = currently_down;
        prev_state_[name] = currently_down;
    }
}

bool KeybindManager::is_just_pressed(const std::string& name) const {
    auto it = binds_.find(name);
    if (it == binds_.end()) return false;

    auto prev_it = prev_state_.find(name);
    bool was_down = (prev_it != prev_state_.end()) ? prev_it->second : false;

    return it->second.active && !was_down;
}

bool KeybindManager::is_held(const std::string& name) const {
    auto it = binds_.find(name);
    return (it != binds_.end()) ? it->second.active : false;
}

void KeybindManager::start_capture(const std::string& target_bind) {
    capturing_ = true;
    capture_target_ = target_bind;
}

void KeybindManager::cancel_capture() {
    capturing_ = false;
    capture_target_.clear();
}

std::string KeybindManager::vk_to_string(int vk) {
    // Common keys
    if (vk >= 'A' && vk <= 'Z') return std::string(1, static_cast<char>(vk));
    if (vk >= '0' && vk <= '9') return std::string(1, static_cast<char>(vk));

    // Function keys
    if (vk >= VK_F1 && vk <= VK_F24) {
        return "F" + std::to_string(vk - VK_F1 + 1);
    }

    // Numpad
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) {
        return "Num" + std::to_string(vk - VK_NUMPAD0);
    }

    // Special keys
    switch (vk) {
        case VK_LBUTTON:    return "LMB";
        case VK_RBUTTON:    return "RMB";
        case VK_MBUTTON:    return "MMB";
        case VK_XBUTTON1:   return "Mouse4";
        case VK_XBUTTON2:   return "Mouse5";
        case VK_BACK:       return "Backspace";
        case VK_TAB:        return "Tab";
        case VK_RETURN:     return "Enter";
        case VK_SHIFT:      return "Shift";
        case VK_CONTROL:    return "Ctrl";
        case VK_MENU:       return "Alt";
        case VK_CAPITAL:    return "CapsLock";
        case VK_ESCAPE:     return "Esc";
        case VK_SPACE:      return "Space";
        case VK_PRIOR:      return "PageUp";
        case VK_NEXT:       return "PageDown";
        case VK_END:        return "End";
        case VK_HOME:       return "Home";
        case VK_LEFT:       return "Left";
        case VK_UP:         return "Up";
        case VK_RIGHT:      return "Right";
        case VK_DOWN:       return "Down";
        case VK_INSERT:     return "Insert";
        case VK_DELETE:     return "Delete";
        case VK_OEM_3:      return "~";
        case VK_OEM_MINUS:  return "-";
        case VK_OEM_PLUS:   return "=";
        case VK_OEM_4:      return "[";
        case VK_OEM_6:      return "]";
        case VK_OEM_5:      return "\\";
        case VK_OEM_1:      return ";";
        case VK_OEM_7:      return "'";
        case VK_OEM_COMMA:  return ",";
        case VK_OEM_PERIOD: return ".";
        case VK_OEM_2:      return "/";
        default: {
            std::ostringstream oss;
            oss << "0x" << std::hex << vk;
            return oss.str();
        }
    }
}

} // namespace noqwd
