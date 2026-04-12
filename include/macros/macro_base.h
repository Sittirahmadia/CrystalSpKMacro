#pragma once
// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Macro Base Class                          ║
// ╚══════════════════════════════════════════════════════════╝

#include <string>
#include <atomic>

namespace noqwd {

class MacroBase {
public:
    explicit MacroBase(const std::string& name) : name_(name) {}
    virtual ~MacroBase() = default;

    // Enable / disable
    void enable()  { enabled_ = true; }
    void disable() { enabled_ = false; }
    void toggle()  { enabled_ = !enabled_.load(); }
    bool is_enabled() const { return enabled_; }

    // Execute the macro sequence
    virtual void execute() = 0;

    // Name for UI
    const std::string& name() const { return name_; }

protected:
    std::string       name_;
    std::atomic<bool> enabled_{false};
};

} // namespace noqwd
