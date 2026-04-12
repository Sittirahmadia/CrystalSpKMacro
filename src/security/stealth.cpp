// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Stealth / Anti-Detection Module           ║
// ║  External-only techniques to minimise detection          ║
// ╚══════════════════════════════════════════════════════════╝

#include "security/stealth.h"
#include <random>
#include <dwmapi.h>
#include <winternl.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "ntdll.lib")

namespace noqwd {

bool Stealth::initialized_ = false;

void Stealth::init() {
    if (initialized_) return;

    set_timer_resolution();
    erase_pe_header();

    initialized_ = true;
}

// ─── Random Window Class Name ───────────────────────────
std::wstring Stealth::generate_random_class_name() {
    static const wchar_t charset[] =
        L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, (int)wcslen(charset) - 1);

    std::wstring name;
    int len = 8 + (rng() % 8);  // 8-15 chars
    for (int i = 0; i < len; ++i) {
        name += charset[dist(rng)];
    }
    return name;
}

// ─── Debug Privilege ────────────────────────────────────
void Stealth::set_debug_privilege() {
    HANDLE token;
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
        return;

    LUID luid;
    if (!LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &luid)) {
        CloseHandle(token);
        return;
    }

    TOKEN_PRIVILEGES tp{};
    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), nullptr, nullptr);
    CloseHandle(token);
}

// ─── Erase PE Header ───────────────────────────────────
void Stealth::erase_pe_header() {
    // Overwrite the PE header in memory to hinder static analysis
    HMODULE base = GetModuleHandleW(nullptr);
    if (!base) return;

    DWORD old_protect;
    if (VirtualProtect(base, 4096, PAGE_READWRITE, &old_protect)) {
        SecureZeroMemory(base, 4096);
        VirtualProtect(base, 4096, old_protect, &old_protect);
    }
}

// ─── DWM Window Cloaking ────────────────────────────────
void Stealth::cloak_window(HWND hwnd) {
    BOOL cloaked = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloaked, sizeof(cloaked));
}

void Stealth::uncloak_window(HWND hwnd) {
    BOOL cloaked = FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloaked, sizeof(cloaked));
}

// ─── Module Name Spoofing ───────────────────────────────
void Stealth::spoof_module_name(const std::wstring& fake_name) {
    // Access the PEB (Process Environment Block) to rename module
    // This uses documented NtQueryInformationProcess or direct TEB access

#if defined(_WIN64)
    // 64-bit: PEB is at TEB + 0x60
    PPEB peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
#else
    // 32-bit: PEB is at TEB + 0x30
    PPEB peb = reinterpret_cast<PPEB>(__readfsdword(0x30));
#endif

    if (!peb || !peb->Ldr) return;

    // Walk the InLoadOrderModuleList
    PLIST_ENTRY head = &peb->Ldr->InMemoryOrderModuleList;
    PLIST_ENTRY curr = head->Flink;

    while (curr != head) {
        PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(
            curr, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        // First entry is usually the main executable
        if (entry->FullDllName.Buffer && entry->BaseDllName.Buffer) {
            // Overwrite the base DLL name with fake name
            size_t copy_len = (fake_name.size() < entry->BaseDllName.MaximumLength / sizeof(wchar_t))
                ? fake_name.size()
                : (entry->BaseDllName.MaximumLength / sizeof(wchar_t) - 1);

            wmemcpy(entry->BaseDllName.Buffer, fake_name.c_str(), copy_len);
            entry->BaseDllName.Buffer[copy_len] = L'\0';
            entry->BaseDllName.Length = static_cast<USHORT>(copy_len * sizeof(wchar_t));
            break;  // Only spoof the main module
        }
        curr = curr->Flink;
    }
}

// ─── Timer Resolution ───────────────────────────────────
typedef LONG(NTAPI* NtSetTimerResolutionFn)(ULONG, BOOLEAN, PULONG);

void Stealth::set_timer_resolution() {
    // Request 0.5ms timer resolution for precision sleep
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return;

    auto NtSetTimerResolution = reinterpret_cast<NtSetTimerResolutionFn>(
        GetProcAddress(ntdll, "NtSetTimerResolution"));

    if (NtSetTimerResolution) {
        ULONG actual;
        NtSetTimerResolution(5000, TRUE, &actual);  // 5000 = 0.5ms in 100ns units
    }
}

void Stealth::restore_timer_resolution() {
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return;

    auto NtSetTimerResolution = reinterpret_cast<NtSetTimerResolutionFn>(
        GetProcAddress(ntdll, "NtSetTimerResolution"));

    if (NtSetTimerResolution) {
        ULONG actual;
        NtSetTimerResolution(156250, TRUE, &actual);  // Default ~15.6ms
    }
}

} // namespace noqwd
