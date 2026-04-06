// ─────────────────────────────────────────────────────────────────────────────
//  optimizer.cpp — Windows OS-level optimizations (native C++ version)
//
//  Registry-based optimizations with backup/restore.  All changes are
//  reversible.  Non-Windows builds compile as no-ops.
// ─────────────────────────────────────────────────────────────────────────────

#include "optimizer.h"
#include <set>
#include <unordered_map>
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <winreg.h>
#include <tlhelp32.h>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "winmm.lib")
#endif

namespace optimizer {

static std::set<std::string> g_applied;

// Backup storage: optKey → vector of {hive, subkey, valueName, originalData, originalType}
struct RegBackup {
    std::string fullPath;  // e.g. "HKCU\\Control Panel\\Keyboard"
    std::string valueName;
    std::string originalData;
    DWORD originalType = 0;
    bool existed = false;
};
static std::unordered_map<std::string, std::vector<RegBackup>> g_backups;

// Timer resolution state
static bool g_timerActive = false;

// Priority polling
static bool g_priorityActive = false;

static const std::vector<std::string> g_allKeys = {
    "keyrepeat", "stickykeys", "accel", "rawinput",
    "priority", "fullscreen", "timer", "network",
    "gamedvr", "powerplan", "visualfx", "gpusched"
};

const std::vector<std::string>& allKeys() { return g_allKeys; }

// ── Registry helpers ────────────────────────────────────────────────────────

#ifdef _WIN32

static HKEY parseHive(const std::string& path, std::string& subkey) {
    if (path.rfind("HKLM\\", 0) == 0) { subkey = path.substr(5); return HKEY_LOCAL_MACHINE; }
    if (path.rfind("HKCU\\", 0) == 0) { subkey = path.substr(5); return HKEY_CURRENT_USER; }
    subkey = path;
    return HKEY_CURRENT_USER;
}

static bool regGet(const std::string& fullPath, const std::string& valueName,
                   std::string& data, DWORD& type) {
    std::string subkey;
    HKEY hive = parseHive(fullPath, subkey);
    HKEY hk;
    if (RegOpenKeyExA(hive, subkey.c_str(), 0, KEY_READ, &hk) != ERROR_SUCCESS)
        return false;

    char buf[1024] = {};
    DWORD bufSize = sizeof(buf);
    type = 0;
    LONG r = RegQueryValueExA(hk, valueName.c_str(), nullptr, &type, (BYTE*)buf, &bufSize);
    RegCloseKey(hk);
    if (r != ERROR_SUCCESS) return false;

    if (type == REG_DWORD && bufSize == 4) {
        DWORD val = *(DWORD*)buf;
        data = std::to_string(val);
    } else {
        data = std::string(buf, bufSize > 0 ? bufSize - 1 : 0);
    }
    return true;
}

static bool regSetStr(const std::string& fullPath, const std::string& valueName,
                      const std::string& data) {
    std::string subkey;
    HKEY hive = parseHive(fullPath, subkey);
    HKEY hk;
    if (RegCreateKeyExA(hive, subkey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hk, nullptr) != ERROR_SUCCESS)
        return false;
    LONG r = RegSetValueExA(hk, valueName.c_str(), 0, REG_SZ, (const BYTE*)data.c_str(), (DWORD)data.size() + 1);
    RegCloseKey(hk);
    return r == ERROR_SUCCESS;
}

static bool regSetDword(const std::string& fullPath, const std::string& valueName, DWORD val) {
    std::string subkey;
    HKEY hive = parseHive(fullPath, subkey);
    HKEY hk;
    if (RegCreateKeyExA(hive, subkey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hk, nullptr) != ERROR_SUCCESS)
        return false;
    LONG r = RegSetValueExA(hk, valueName.c_str(), 0, REG_DWORD, (const BYTE*)&val, sizeof(val));
    RegCloseKey(hk);
    return r == ERROR_SUCCESS;
}

static bool regDelete(const std::string& fullPath, const std::string& valueName) {
    std::string subkey;
    HKEY hive = parseHive(fullPath, subkey);
    HKEY hk;
    if (RegOpenKeyExA(hive, subkey.c_str(), 0, KEY_WRITE, &hk) != ERROR_SUCCESS) return false;
    LONG r = RegDeleteValueA(hk, valueName.c_str());
    RegCloseKey(hk);
    return r == ERROR_SUCCESS;
}

static void backupEntry(const std::string& optKey, const std::string& fullPath, const std::string& valueName) {
    RegBackup bk;
    bk.fullPath = fullPath;
    bk.valueName = valueName;
    bk.existed = regGet(fullPath, valueName, bk.originalData, bk.originalType);
    g_backups[optKey].push_back(bk);
}

static void restoreBackup(const std::string& optKey) {
    auto it = g_backups.find(optKey);
    if (it == g_backups.end()) return;
    for (auto& bk : it->second) {
        if (bk.existed) {
            if (bk.originalType == REG_DWORD) {
                regSetDword(bk.fullPath, bk.valueName, (DWORD)std::stoul(bk.originalData));
            } else {
                regSetStr(bk.fullPath, bk.valueName, bk.originalData);
            }
        } else {
            regDelete(bk.fullPath, bk.valueName);
        }
    }
    g_backups.erase(it);
}

// ── Run command helper ──────────────────────────────────────────────────────

static bool runCmd(const std::string& cmd) {
    STARTUPINFOA si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = {};

    std::string cmdLine = "cmd.exe /c " + cmd;
    char buf[4096];
    strncpy_s(buf, cmdLine.c_str(), sizeof(buf) - 1);

    if (!CreateProcessA(nullptr, buf, nullptr, nullptr, FALSE,
                        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
        return false;

    WaitForSingleObject(pi.hProcess, 10000);
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exitCode == 0;
}

// ── Enumerate TCP interfaces ────────────────────────────────────────────────

static std::vector<std::string> enumTcpInterfaces() {
    std::vector<std::string> result;
    const char* base = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces";
    HKEY hk;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, base, 0, KEY_READ, &hk) != ERROR_SUCCESS) return result;

    char name[256];
    for (DWORD i = 0; ; i++) {
        DWORD nameLen = sizeof(name);
        if (RegEnumKeyExA(hk, i, name, &nameLen, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
        result.push_back(std::string("HKLM\\") + base + "\\" + name);
    }
    RegCloseKey(hk);
    return result;
}

// ── Set process priority ────────────────────────────────────────────────────

static void setMcPriority(DWORD priorityClass) {
    // Use toolhelp to find javaw.exe
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe = { sizeof(pe) };
    if (Process32FirstW(snap, &pe)) {
        do {
            std::wstring name(pe.szExeFile);
            // Check for javaw.exe or java.exe (Minecraft)
            if (name == L"javaw.exe" || name == L"java.exe") {
                HANDLE proc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe.th32ProcessID);
                if (proc) {
                    SetPriorityClass(proc, priorityClass);
                    CloseHandle(proc);
                }
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
}

#endif // _WIN32

// ── Optimization implementations ────────────────────────────────────────────

static Result applyKeyrepeat() {
#ifdef _WIN32
    const std::string kb = "HKCU\\Control Panel\\Keyboard";
    backupEntry("keyrepeat", kb, "KeyboardDelay");
    backupEntry("keyrepeat", kb, "KeyboardSpeed");
    regSetStr(kb, "KeyboardDelay", "0");
    regSetStr(kb, "KeyboardSpeed", "31");
    // Apply immediately via SystemParametersInfo
    SystemParametersInfoW(SPI_SETKEYBOARDDELAY, 0, nullptr, SPIF_SENDCHANGE);
    SystemParametersInfoW(SPI_SETKEYBOARDSPEED, 31, nullptr, SPIF_SENDCHANGE);
    return {true, "", "applied"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertKeyrepeat() {
#ifdef _WIN32
    auto& bks = g_backups["keyrepeat"];
    UINT delay = 1, speed = 20;
    if (bks.size() >= 2) {
        try { delay = std::stoul(bks[0].originalData); } catch (...) {}
        try { speed = std::stoul(bks[1].originalData); } catch (...) {}
    }
    restoreBackup("keyrepeat");
    SystemParametersInfoW(SPI_SETKEYBOARDDELAY, delay, nullptr, SPIF_SENDCHANGE);
    SystemParametersInfoW(SPI_SETKEYBOARDSPEED, speed, nullptr, SPIF_SENDCHANGE);
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result applyStickykeys() {
#ifdef _WIN32
    const std::string sk = "HKCU\\Control Panel\\Accessibility\\StickyKeys";
    const std::string fk = "HKCU\\Control Panel\\Accessibility\\Keyboard Response";
    const std::string tk = "HKCU\\Control Panel\\Accessibility\\ToggleKeys";
    backupEntry("stickykeys", sk, "Flags");
    backupEntry("stickykeys", fk, "Flags");
    backupEntry("stickykeys", tk, "Flags");
    regSetStr(sk, "Flags", "506");
    regSetStr(fk, "Flags", "122");
    regSetStr(tk, "Flags", "58");

    // Apply immediately
    STICKYKEYS sks = { sizeof(STICKYKEYS), 506 };
    SystemParametersInfoW(SPI_SETSTICKYKEYS, sizeof(sks), &sks, SPIF_SENDCHANGE);
    FILTERKEYS fks = { sizeof(FILTERKEYS) }; fks.dwFlags = 122;
    SystemParametersInfoW(SPI_SETFILTERKEYS, sizeof(fks), &fks, SPIF_SENDCHANGE);
    TOGGLEKEYS tks = { sizeof(TOGGLEKEYS), 58 };
    SystemParametersInfoW(SPI_SETTOGGLEKEYS, sizeof(tks), &tks, SPIF_SENDCHANGE);

    return {true, "", "applied"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertStickykeys() {
#ifdef _WIN32
    restoreBackup("stickykeys");
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result applyAccel() {
#ifdef _WIN32
    const std::string m = "HKCU\\Control Panel\\Mouse";
    backupEntry("accel", m, "MouseSpeed");
    backupEntry("accel", m, "MouseThreshold1");
    backupEntry("accel", m, "MouseThreshold2");
    regSetStr(m, "MouseSpeed", "0");
    regSetStr(m, "MouseThreshold1", "0");
    regSetStr(m, "MouseThreshold2", "0");
    int params[3] = {0, 0, 0};
    SystemParametersInfoW(SPI_SETMOUSE, 0, params, SPIF_SENDCHANGE);
    return {true, "", "applied"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertAccel() {
#ifdef _WIN32
    auto& bks = g_backups["accel"];
    int params[3] = {1, 6, 10};
    if (bks.size() >= 3) {
        try { params[0] = std::stoi(bks[0].originalData); } catch (...) {}
        try { params[1] = std::stoi(bks[1].originalData); } catch (...) {}
        try { params[2] = std::stoi(bks[2].originalData); } catch (...) {}
    }
    restoreBackup("accel");
    SystemParametersInfoW(SPI_SETMOUSE, 0, params, SPIF_SENDCHANGE);
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result applyRawinput() {
#ifdef _WIN32
    const std::string m = "HKCU\\Control Panel\\Mouse";
    backupEntry("rawinput", m, "MouseSensitivity");
    regSetStr(m, "MouseSensitivity", "10");
    SystemParametersInfoW(SPI_SETMOUSESPEED, 0, (PVOID)10, SPIF_SENDCHANGE);
    return {true, "", "applied"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertRawinput() {
#ifdef _WIN32
    auto& bks = g_backups["rawinput"];
    int sens = 10;
    if (!bks.empty()) { try { sens = std::stoi(bks[0].originalData); } catch (...) {} }
    restoreBackup("rawinput");
    SystemParametersInfoW(SPI_SETMOUSESPEED, 0, (PVOID)(intptr_t)sens, SPIF_SENDCHANGE);
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result applyPriority() {
#ifdef _WIN32
    setMcPriority(HIGH_PRIORITY_CLASS);
    g_priorityActive = true;
    return {true, "", "applied"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertPriority() {
#ifdef _WIN32
    g_priorityActive = false;
    setMcPriority(NORMAL_PRIORITY_CLASS);
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result applyFullscreen(const std::string& mcExe) {
#ifdef _WIN32
    if (mcExe.empty()) return {false, "no-mc-path", ""};
    std::string exePath = std::filesystem::absolute(mcExe).string();
    const std::string layers = "HKCU\\Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers";
    backupEntry("fullscreen", layers, exePath);
    regSetStr(layers, exePath, "~ DISABLEDXMAXIMIZEDWINDOWEDMODE");
    return {true, "", "applied"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertFullscreen() {
#ifdef _WIN32
    restoreBackup("fullscreen");
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result applyTimer() {
#ifdef _WIN32
    if (g_timerActive) return {true, "", "already-active"};
    timeBeginPeriod(1);
    g_timerActive = true;
    return {true, "", "applied"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertTimer() {
#ifdef _WIN32
    if (g_timerActive) {
        timeEndPeriod(1);
        g_timerActive = false;
    }
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result applyNetwork() {
#ifdef _WIN32
    auto ifaces = enumTcpInterfaces();
    if (ifaces.empty()) return {false, "No network interfaces found", ""};

    for (auto& iface : ifaces) {
        backupEntry("network", iface, "TcpAckFrequency");
        backupEntry("network", iface, "TCPNoDelay");
    }

    int applied = 0;
    for (auto& iface : ifaces) {
        bool ok1 = regSetDword(iface, "TcpAckFrequency", 1);
        bool ok2 = regSetDword(iface, "TCPNoDelay", 1);
        if (ok1 && ok2) applied++;
    }

    if (applied == 0) {
        g_backups.erase("network");
        return {false, "Access denied - run as administrator", ""};
    }

    return {true, "", "applied"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertNetwork() {
#ifdef _WIN32
    restoreBackup("network");
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result applyGamedvr() {
#ifdef _WIN32
    const std::string dvr = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\GameDVR";
    const std::string bar = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\GameBar";
    const std::string ldvr = "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\GameDVR";

    backupEntry("gamedvr", dvr, "AppCaptureEnabled");
    backupEntry("gamedvr", bar, "AllowAutoGameMode");
    backupEntry("gamedvr", bar, "AutoGameModeEnabled");
    backupEntry("gamedvr", bar, "ShowStartupPanel");
    backupEntry("gamedvr", bar, "UseNexusForGameBarEnabled");
    backupEntry("gamedvr", ldvr, "AllowGameDVR");

    regSetDword(dvr, "AppCaptureEnabled", 0);
    regSetDword(bar, "AllowAutoGameMode", 0);
    regSetDword(bar, "AutoGameModeEnabled", 0);
    regSetDword(bar, "ShowStartupPanel", 0);
    regSetDword(bar, "UseNexusForGameBarEnabled", 0);
    regSetDword(ldvr, "AllowGameDVR", 0);

    return {true, "", "applied"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertGamedvr() {
#ifdef _WIN32
    restoreBackup("gamedvr");
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result applyPowerplan() {
#ifdef _WIN32
    // Backup current plan
    g_backups["powerplan"] = {}; // placeholder
    // Get current active scheme GUID via powercfg
    // High Performance GUID: 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c
    runCmd("powercfg /duplicatescheme 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c");
    bool ok = runCmd("powercfg /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c");
    if (!ok) return {false, "Failed to switch power plan", ""};
    return {true, "", "applied"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertPowerplan() {
#ifdef _WIN32
    // Fall back to Balanced
    runCmd("powercfg /setactive 381b4222-f694-41f0-9685-ff5bb260df2e");
    g_backups.erase("powerplan");
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result applyVisualfx() {
#ifdef _WIN32
    const std::string vfx = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VisualEffects";
    const std::string desk = "HKCU\\Control Panel\\Desktop";
    const std::string dwm = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
    const std::string wdwm = "HKCU\\Software\\Microsoft\\Windows\\DWM";

    backupEntry("visualfx", vfx, "VisualFXSetting");
    backupEntry("visualfx", desk, "DragFullWindows");
    backupEntry("visualfx", dwm, "EnableTransparency");
    backupEntry("visualfx", wdwm, "EnableAeroPeek");

    regSetDword(vfx, "VisualFXSetting", 2);
    regSetStr(desk, "DragFullWindows", "0");
    regSetDword(dwm, "EnableTransparency", 0);
    regSetDword(wdwm, "EnableAeroPeek", 0);

    // Apply immediately
    SystemParametersInfoW(SPI_SETDRAGFULLWINDOWS, 0, nullptr, SPIF_SENDCHANGE);
    SystemParametersInfoW(0x1043 /* SPI_SETCLIENTAREAANIMATION */, 0, nullptr, SPIF_SENDCHANGE);

    return {true, "", "applied"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertVisualfx() {
#ifdef _WIN32
    restoreBackup("visualfx");
    SystemParametersInfoW(SPI_SETDRAGFULLWINDOWS, 1, nullptr, SPIF_SENDCHANGE);
    SystemParametersInfoW(0x1043, 0, (PVOID)1, SPIF_SENDCHANGE);
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result applyGpusched() {
#ifdef _WIN32
    const std::string gfx = "HKLM\\SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers";
    backupEntry("gpusched", gfx, "HwSchMode");
    bool ok = regSetDword(gfx, "HwSchMode", 2);
    if (!ok) return {false, "Failed - run as administrator", ""};
    return {true, "", "applied-reboot"};
#else
    return {true, "", "not-windows"};
#endif
}

static Result revertGpusched() {
#ifdef _WIN32
    restoreBackup("gpusched");
    return {true, "", "reverted"};
#else
    return {true, "", "not-windows"};
#endif
}

// ── Dispatch table ──────────────────────────────────────────────────────────

using OptFn = Result(*)(const std::string&);
using OptFnSimple = Result(*)();

struct OptEntry {
    std::string key;
    Result (*applyFn)(const std::string&);
    Result (*revertFn)();
};

// Wrapper to unify signatures
static Result wrapApply(const std::string& key, const std::string& mcExe) {
    if (key == "keyrepeat")  return applyKeyrepeat();
    if (key == "stickykeys") return applyStickykeys();
    if (key == "accel")      return applyAccel();
    if (key == "rawinput")   return applyRawinput();
    if (key == "priority")   return applyPriority();
    if (key == "fullscreen") return applyFullscreen(mcExe);
    if (key == "timer")      return applyTimer();
    if (key == "network")    return applyNetwork();
    if (key == "gamedvr")    return applyGamedvr();
    if (key == "powerplan")  return applyPowerplan();
    if (key == "visualfx")   return applyVisualfx();
    if (key == "gpusched")   return applyGpusched();
    return {false, "Unknown optimization: " + key, ""};
}

static Result wrapRevert(const std::string& key) {
    if (key == "keyrepeat")  return revertKeyrepeat();
    if (key == "stickykeys") return revertStickykeys();
    if (key == "accel")      return revertAccel();
    if (key == "rawinput")   return revertRawinput();
    if (key == "priority")   return revertPriority();
    if (key == "fullscreen") return revertFullscreen();
    if (key == "timer")      return revertTimer();
    if (key == "network")    return revertNetwork();
    if (key == "gamedvr")    return revertGamedvr();
    if (key == "powerplan")  return revertPowerplan();
    if (key == "visualfx")   return revertVisualfx();
    if (key == "gpusched")   return revertGpusched();
    return {false, "Unknown optimization: " + key, ""};
}

// ── Public API ──────────────────────────────────────────────────────────────

Result applyOpt(const std::string& key, const std::string& mcExePath) {
    Result r = wrapApply(key, mcExePath);
    if (r.ok) {
        g_applied.insert(key);
        std::cout << "[optimizer] Applied: " << key << " (" << r.mode << ")\n";
    } else {
        std::cerr << "[optimizer] Failed: " << key << " - " << r.error << "\n";
    }
    return r;
}

Result revertOpt(const std::string& key, const std::string& mcExePath) {
    Result r = wrapRevert(key);
    if (r.ok) {
        g_applied.erase(key);
        std::cout << "[optimizer] Reverted: " << key << "\n";
    }
    return r;
}

std::vector<std::string> getApplied() {
    return std::vector<std::string>(g_applied.begin(), g_applied.end());
}

void restoreApplied(const std::vector<std::string>& keys, const std::string& mcExePath) {
    for (auto& k : keys) {
        applyOpt(k, mcExePath);
    }
}

void cleanup() {
    std::cout << "[optimizer] Cleaning up...\n";
    for (auto& k : std::vector<std::string>(g_applied.begin(), g_applied.end())) {
        revertOpt(k);
    }
    g_applied.clear();
    g_backups.clear();
    std::cout << "[optimizer] Cleanup complete\n";
}

} // namespace optimizer
