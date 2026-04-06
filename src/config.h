#pragma once
// ─────────────────────────────────────────────────────────────────────────────
//  config.h — JSON config structures for CrystalSpKMacro native
// ─────────────────────────────────────────────────────────────────────────────

#include <string>
#include <unordered_map>
#include <vector>

// Per-macro configuration
struct MacroConfig {
    bool active = false;
    std::string keybind = "None";
    int delay = 30;
    std::unordered_map<std::string, std::string> slots; // e.g. anchorKey→"4"
};

// App-wide settings
struct AppSettings {
    bool focusLock = true;
    bool streamProof = false;
    std::string chatKey = "T";
    int chatTimer = 10;
    std::string mcExePath;
    std::string leftClickBind = "Mouse1";
    std::string rightClickBind = "Mouse2";
    std::vector<std::string> appliedOpts;
    std::string theme = "dark";
};

// Full config
struct Config {
    std::unordered_map<std::string, MacroConfig> macros;
    AppSettings settings;
};

namespace config {

// Default macro definitions (id → display name, default delay, slot keys)
struct MacroDef {
    std::string id;
    std::string name;
    std::string category; // "crystal", "sword", "mace", "cart", "uhc"
    int defaultDelay;
    std::vector<std::string> slotNames; // ordered slot key names
};

// Get all macro definitions
const std::vector<MacroDef>& getAllMacroDefs();

// Load config from JSON file.  Returns default config if file missing.
Config load(const std::string& path);

// Save config to JSON file.
void save(const std::string& path, const Config& cfg);

// Get default config path (%APPDATA%/CrystalSpKMacro/config.json)
std::string getDefaultPath();

} // namespace config
