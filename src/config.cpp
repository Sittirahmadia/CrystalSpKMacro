// ─────────────────────────────────────────────────────────────────────────────
//  config.cpp — JSON config load/save using nlohmann/json
// ─────────────────────────────────────────────────────────────────────────────

#include "config.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace config {

// ── Macro definitions ───────────────────────────────────────────────────────

static const std::vector<MacroDef> g_defs = {
    {"sa",  "Single Anchor",        "crystal", 27,  {"anchorKey", "glowstoneKey", "explodeKey"}},
    {"da",  "Double Anchor",        "crystal", 48,  {"anchorKey", "glowstoneKey"}},
    {"ap",  "Anchor Pearl",         "crystal", 25,  {"anchorKey", "glowstoneKey", "explodeKey", "pearlKey", "totemKey"}},
    {"hc",  "Hit Crystal",          "crystal", 1,   {"obsidianKey", "crystalKey"}},
    {"shc", "Slow Hit Crystal",     "crystal", 30,  {"obsidianKey", "crystalKey"}},
    {"ac",  "Auto Crystal",         "crystal", 25,  {"crystalKey"}},
    {"kp",  "Key Pearl",            "crystal", 30,  {"pearlKey", "returnKey"}},
    {"idh", "Inventory D-Hand",     "crystal", 25,  {"inventoryKey", "totemKey"}},
    {"oht", "Offhand Totem",        "crystal", 35,  {"totemKey", "swapKey"}},
    {"fxp", "Fast XP",              "crystal", 35,  {}},
    {"asb", "Auto Shield Breaker",  "sword",   35,  {"axeKey", "swordKey"}},
    {"ls",  "Lunge Swap",           "sword",   0,   {"swordKey", "spearKey"}},
    {"es",  "Elytra Swap",          "mace",    50,  {"elytraKey", "returnKey"}},
    {"pc",  "Pearl Catch",          "mace",    50,  {"pearlKey", "windChargeKey"}},
    {"ss",  "Stun Slam",            "mace",    10,  {"axeKey", "maceKey"}},
    {"bs",  "Breach Swap",          "mace",    25,  {"maceKey", "swordKey"}},
    {"ic",  "Insta Cart",           "cart",    50,  {"railKey", "bowKey", "cartKey", "bowHoldMs"}},
    {"xb",  "Crossbow Cart",        "cart",    50,  {"railKey", "cartKey", "fnsKey", "crossbowKey"}},
    {"dr",  "Drain",                "uhc",     30,  {"bucketKey"}},
    {"lw",  "Lava Web",             "uhc",     30,  {"lavaKey", "cobwebKey"}},
    {"la",  "Lava",                 "uhc",     30,  {"lavaKey"}},
};

const std::vector<MacroDef>& getAllMacroDefs() {
    return g_defs;
}

// ── Default config path ─────────────────────────────────────────────────────

std::string getDefaultPath() {
#ifdef _WIN32
    wchar_t* appData = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appData))) {
        fs::path p = fs::path(appData) / "CrystalSpKMacro";
        CoTaskMemFree(appData);
        fs::create_directories(p);
        return (p / "config.json").string();
    }
#endif
    return "config.json";
}

// ── Load ────────────────────────────────────────────────────────────────────

Config load(const std::string& path) {
    Config cfg;

    // Initialize all macros with defaults
    for (auto& def : g_defs) {
        MacroConfig mc;
        mc.delay = def.defaultDelay;
        cfg.macros[def.id] = mc;
    }

    // Try to load from file
    if (!fs::exists(path)) return cfg;

    try {
        std::ifstream f(path);
        json j = json::parse(f);

        // Load macros
        if (j.contains("macros") && j["macros"].is_object()) {
            for (auto& [id, jm] : j["macros"].items()) {
                if (cfg.macros.find(id) == cfg.macros.end()) continue;
                auto& mc = cfg.macros[id];

                if (jm.contains("active"))  mc.active  = jm["active"].get<bool>();
                if (jm.contains("keybind")) mc.keybind = jm["keybind"].get<std::string>();
                if (jm.contains("delay"))   mc.delay   = jm["delay"].get<int>();

                if (jm.contains("slots") && jm["slots"].is_object()) {
                    for (auto& [sk, sv] : jm["slots"].items()) {
                        mc.slots[sk] = sv.get<std::string>();
                    }
                }
            }
        }

        // Load settings
        if (j.contains("settings") && j["settings"].is_object()) {
            auto& js = j["settings"];
            auto& s = cfg.settings;
            if (js.contains("focusLock"))      s.focusLock      = js["focusLock"].get<bool>();
            if (js.contains("streamProof"))    s.streamProof    = js["streamProof"].get<bool>();
            if (js.contains("chatKey"))        s.chatKey        = js["chatKey"].get<std::string>();
            if (js.contains("chatTimer"))      s.chatTimer      = js["chatTimer"].get<int>();
            if (js.contains("mcExePath"))      s.mcExePath      = js["mcExePath"].get<std::string>();
            if (js.contains("leftClickBind"))  s.leftClickBind  = js["leftClickBind"].get<std::string>();
            if (js.contains("rightClickBind")) s.rightClickBind = js["rightClickBind"].get<std::string>();
            if (js.contains("theme"))          s.theme          = js["theme"].get<std::string>();
            if (js.contains("appliedOpts") && js["appliedOpts"].is_array()) {
                s.appliedOpts.clear();
                for (auto& v : js["appliedOpts"]) {
                    s.appliedOpts.push_back(v.get<std::string>());
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[config] Parse error: " << e.what() << "\n";
    }

    return cfg;
}

// ── Save ────────────────────────────────────────────────────────────────────

void save(const std::string& path, const Config& cfg) {
    json j;

    // Macros
    json jm;
    for (auto& [id, mc] : cfg.macros) {
        json entry;
        entry["active"]  = mc.active;
        entry["keybind"] = mc.keybind;
        entry["delay"]   = mc.delay;

        json slots;
        for (auto& [k, v] : mc.slots) {
            slots[k] = v;
        }
        entry["slots"] = slots;
        jm[id] = entry;
    }
    j["macros"] = jm;

    // Settings
    json js;
    js["focusLock"]      = cfg.settings.focusLock;
    js["streamProof"]    = cfg.settings.streamProof;
    js["chatKey"]        = cfg.settings.chatKey;
    js["chatTimer"]      = cfg.settings.chatTimer;
    js["mcExePath"]      = cfg.settings.mcExePath;
    js["leftClickBind"]  = cfg.settings.leftClickBind;
    js["rightClickBind"] = cfg.settings.rightClickBind;
    js["theme"]          = cfg.settings.theme;
    js["appliedOpts"]    = cfg.settings.appliedOpts;
    j["settings"] = js;

    // Write
    try {
        fs::path p(path);
        fs::create_directories(p.parent_path());
        std::ofstream f(path);
        f << j.dump(2) << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "[config] Save error: " << e.what() << "\n";
    }
}

} // namespace config
