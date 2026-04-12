// ╔══════════════════════════════════════════════════════════╗
// ║  noqwdmacro — Stealth / Anti-Detection Module           ║
// ║  External-only techniques to minimise detection          ║
// ╚══════════════════════════════════════════════════════════╝

#include "security/stealth.h"
#include <random>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "ntdll.lib")

// ─── Custom PEB structures (SDK winternl.h is incomplete) ──
// We define our own full version to access BaseDllName
typedef struct _NOQWD_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} NOQWD_UNICODE_STRING;

typedef struct _NOQWD_LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID      DllBase;
    PVOID      EntryPoint;
    ULONG      SizeOfImage;
    NOQWD_UNICODE_STRING FullDllName;
    NOQWD_UNICODE_STRING BaseDllName;
    // ... more fields follow but we don't need them
} NOQWD_LDR_DATA_TABLE_ENTRY;

typedef struct _NOQWD_PEB_LDR_DATA {
    ULONG      Length;
    BOOLEAN    Initialized;
    HANDLE     SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} NOQWD_PEB_LDR_DATA;

typedef struct _NOQWD_PEB {
    BYTE                 Reserved1[2];
    BYTE                 BeingDebugged;
    BYTE                 Reserved2[1];
    PVOID                Reserved3[2];
    NOQWD_PEB_LDR_DATA* Ldr;
    // ... more fields follow
} NOQWD_PEB;

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
    // Access PEB directly via TEB to walk the module list

#if defined(_WIN64)
    NOQWD_PEB* peb = reinterpret_cast<NOQWD_PEB*>(__readgsqword(0x60));
#else
    NOQWD_PEB* peb = reinterpret_cast<NOQWD_PEB*>(__readfsdword(0x30));
#endif

    if (!peb || !peb->Ldr) return;

    // Walk the InLoadOrderModuleList
    PLIST_ENTRY head = &peb->Ldr->InLoadOrderModuleList;
    PLIST_ENTRY curr = head->Flink;

    while (curr != head) {
        auto* entry = CONTAINING_RECORD(
            curr, NOQWD_LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        // First entry is usually the main executable
        if (entry->FullDllName.Buffer && entry->BaseDllName.Buffer) {
            size_t max_chars = entry->BaseDllName.MaximumLength / sizeof(wchar_t);
            size_t copy_len = (fake_name.size() < max_chars)
                ? fake_name.size()
                : (max_chars - 1);

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
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return;

    auto NtSetTimerResolution = reinterpret_cast<NtSetTimerResolutionFn>(
        GetProcAddress(ntdll, "NtSetTimerResolution"));

    if (NtSetTimerResolution) {
        ULONG actual;
        NtSetTimerResolution(5000, TRUE, &actual);  // 0.5ms
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
