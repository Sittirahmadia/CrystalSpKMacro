#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Keybind Manager                           ║
// ╚══════════════════════════════════════════════════════════╝

#include <cstdint>
#include <string>
#include <functional>
#include <unordered_map>
#include <windows.h>

namespace noqwd {

struct Keybind {
    int         vk_code  = 0;       // Virtual key code (0 = unbound)
    bool        active   = false;   // Currently held down
    std::string label    = "None";  // Display label
};

class KeybindManager {
public:
    static KeybindManager& instance();

    // Register a named keybind
    void   register_bind(const std::string& name, int default_vk = 0);

    // Get / set
    Keybind&       get(const std::string& name);
    const Keybind& get(const std::string& name) const;
    void           set(const std::string& name, int vk_code);

    // Polling (call each frame)
    void update();

    // Check if a bind is currently pressed (rising edge)
    bool is_just_pressed(const std::string& name) const;

    // Check if a bind is held
    bool is_held(const std::string& name) const;

    // Convert VK code → display string
    static std::string vk_to_string(int vk);

    // Capture next key press (for rebinding UI)
    bool is_capturing() const { return capturing_; }
    void start_capture(const std::string& target_bind);
    void cancel_capture();

    const std::unordered_map<std::string, Keybind>& all_binds() const { return binds_; }

private:
    KeybindManager() = default;

    std::unordered_map<std::string, Keybind> binds_;
    std::unordered_map<std::string, bool>    prev_state_;

    bool        capturing_    = false;
    std::string capture_target_;
};

} // namespace noqwd
