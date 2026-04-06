// ─────────────────────────────────────────────────────────────────────────────
//  tray.cpp — System tray icon with context menu
// ─────────────────────────────────────────────────────────────────────────────

#include "tray.h"
#include "resource.h"

#ifdef _WIN32
#include <shellapi.h>

namespace tray {

static NOTIFYICONDATAW g_nid = {};
static HWND g_parentHwnd = nullptr;
static TrayCallbacks g_cbs;

void init(HWND parentHwnd, HINSTANCE hInst, const TrayCallbacks& cbs) {
    g_parentHwnd = parentHwnd;
    g_cbs = cbs;

    ZeroMemory(&g_nid, sizeof(g_nid));
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = parentHwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_APP_ICON));

    // Fallback to default app icon if resource not found
    if (!g_nid.hIcon) {
        g_nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    }

    wcscpy_s(g_nid.szTip, L"CrystalSpKMacro");
    Shell_NotifyIconW(NIM_ADD, &g_nid);
}

void handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg != WM_TRAYICON) return;

    switch (LOWORD(lParam)) {
        case WM_LBUTTONDBLCLK:
            if (g_cbs.onShow) g_cbs.onShow();
            break;

        case WM_RBUTTONUP: {
            POINT pt;
            GetCursorPos(&pt);

            HMENU menu = CreatePopupMenu();
            AppendMenuW(menu, MF_STRING, IDM_TRAY_SHOW,    L"Show");
            AppendMenuW(menu, MF_STRING, IDM_TRAY_HIDE,    L"Hide");
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
            AppendMenuW(menu, MF_STRING, IDM_TRAY_STOPALL, L"Stop All Macros");
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
            AppendMenuW(menu, MF_STRING, IDM_TRAY_EXIT,    L"Exit");

            // Required for menu to close when clicking elsewhere
            SetForegroundWindow(hwnd);
            UINT cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY,
                                      pt.x, pt.y, 0, hwnd, nullptr);
            DestroyMenu(menu);
            PostMessageW(hwnd, WM_NULL, 0, 0);

            switch (cmd) {
                case IDM_TRAY_SHOW:    if (g_cbs.onShow) g_cbs.onShow(); break;
                case IDM_TRAY_HIDE:    if (g_cbs.onHide) g_cbs.onHide(); break;
                case IDM_TRAY_STOPALL: if (g_cbs.onStopAll) g_cbs.onStopAll(); break;
                case IDM_TRAY_EXIT:    if (g_cbs.onExit) g_cbs.onExit(); break;
            }
            break;
        }
    }
}

void cleanup() {
    Shell_NotifyIconW(NIM_DELETE, &g_nid);
}

} // namespace tray
#endif // _WIN32
