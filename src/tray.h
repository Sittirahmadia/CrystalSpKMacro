#pragma once
// ─────────────────────────────────────────────────────────────────────────────
//  tray.h — System tray icon for CrystalSpKMacro native
// ─────────────────────────────────────────────────────────────────────────────

#include <functional>

#ifdef _WIN32
#include <windows.h>
#endif

namespace tray {

struct TrayCallbacks {
    std::function<void()> onShow;
    std::function<void()> onHide;
    std::function<void()> onStopAll;
    std::function<void()> onExit;
};

#ifdef _WIN32
void init(HWND parentHwnd, HINSTANCE hInst, const TrayCallbacks& cbs);
void handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

void cleanup();

} // namespace tray
