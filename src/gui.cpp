// ─────────────────────────────────────────────────────────────────────────────
//  gui.cpp — Win32 dark-themed GUI v2 for CrystalSpKMacro native
//
//  Pixel-matched to the Electron version's CSS with improvements:
//    • True 2-column macro card grid with expandable detail panels
//    • Smooth scrolling with WM_MOUSEWHEEL
//    • Keybind capture overlay with press-any-key prompt
//    • Detection bar with live MC focus LED
//    • Active-macro chip strip
//    • GDI double-buffered painting (zero flicker)
//    • DWM dark title bar integration
// ─────────────────────────────────────────────────────────────────────────────

#include "gui.h"
#include "input.h"
#include "macros.h"
#include "optimizer.h"
#include "resource.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <mutex>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <uxtheme.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "msimg32.lib")

namespace gui {

// ═════════════════════════════════════════════════════════════════════════════
//  COLOR PALETTE — matches Electron :root CSS variables exactly
// ═════════════════════════════════════════════════════════════════════════════

namespace col {
    // Backgrounds (from Electron --bg, --s1, --s2, --s3, --s4)
    constexpr COLORREF BG       = RGB(11, 13, 18);   // #0b0d12
    constexpr COLORREF S1       = RGB(17, 21, 32);   // #111520
    constexpr COLORREF S2       = RGB(24, 28, 40);   // #181c28
    constexpr COLORREF S3       = RGB(31, 35, 52);   // #1f2334
    constexpr COLORREF S4       = RGB(39, 45, 62);   // #272d3e

    // Text (from --t0, --t1, --t2, --t3)
    constexpr COLORREF T0       = RGB(232, 238, 255); // #e8eeff
    constexpr COLORREF T1       = RGB(122, 136, 176); // #7a88b0
    constexpr COLORREF T2       = RGB(65, 77, 106);   // #414d6a
    constexpr COLORREF T3       = RGB(37, 46, 69);    // #252e45

    // Accent (from --green / --accent-rgb)
    constexpr COLORREF ACCENT   = RGB(79, 200, 255);  // #4fc8ff
    constexpr COLORREF GREEN    = RGB(79, 200, 255);  // same as accent
    constexpr COLORREF RED      = RGB(240, 94, 122);  // #f05e7a
    constexpr COLORREF YELLOW   = RGB(240, 192, 64);  // #f0c040

    // Derived
    constexpr COLORREF BORDER       = RGB(25, 33, 55);    // rgba(100,150,255,.09) on BG
    constexpr COLORREF BORDER_HOVER = RGB(40, 48, 72);
    constexpr COLORREF CARD_ACTIVE  = RGB(18, 25, 42);    // accent-faint on S1
    constexpr COLORREF CARD_GLOW    = RGB(20, 32, 55);
    constexpr COLORREF TOGGLE_ON    = RGB(79, 200, 255);
    constexpr COLORREF TOGGLE_OFF   = RGB(39, 45, 62);    // S4
    constexpr COLORREF SIDEBAR_BG   = RGB(14, 18, 28);    // S1 with slight gradient

    // Nav active
    constexpr COLORREF NAV_ACTIVE_BG   = RGB(18, 25, 42);
    constexpr COLORREF NAV_ACTIVE_BAR  = RGB(79, 200, 255);

    // Detection bar
    constexpr COLORREF DET_BG    = RGB(15, 19, 30);
    constexpr COLORREF DET_BORDER= RGB(25, 33, 55);

    // Chip
    constexpr COLORREF CHIP_BG   = RGB(20, 24, 38);
    constexpr COLORREF CHIP_DOT  = RGB(79, 200, 255);

    // Overlay
    constexpr COLORREF OVERLAY   = RGB(8, 10, 14);
}

// ═════════════════════════════════════════════════════════════════════════════
//  LAYOUT CONSTANTS
// ═════════════════════════════════════════════════════════════════════════════

namespace layout {
    constexpr int WIN_W          = 960;
    constexpr int WIN_H          = 680;
    constexpr int SIDEBAR_W      = 220;
    constexpr int TITLEBAR_H     = 44;
    constexpr int DET_BAR_H      = 38;
    constexpr int CHIPS_H        = 32;
    constexpr int CARD_GAP       = 10;
    constexpr int CARD_PAD_X     = 28;
    constexpr int CARD_COLLAPSED = 56;
    constexpr int CARD_FIELD_H   = 34;
    constexpr int CARD_BODY_PAD  = 12;
    constexpr int RADIUS         = 8;
    constexpr int RADIUS_LG      = 12;
    constexpr int TOGGLE_W       = 34;
    constexpr int TOGGLE_H       = 18;
    constexpr int TOGGLE_R       = 9;
    constexpr int TOGGLE_DOT     = 12;
    constexpr int FIELD_INPUT_W  = 80;
    constexpr int FIELD_INPUT_H  = 26;
    constexpr int NAV_ITEM_H     = 36;
    constexpr int NAV_SECTION_H  = 28;
}

// ═════════════════════════════════════════════════════════════════════════════
//  STATE
// ═════════════════════════════════════════════════════════════════════════════

static HWND       g_hwnd = nullptr;
static HINSTANCE  g_hInst = nullptr;
static Config     g_cfg;
static Callbacks  g_cbs;
static std::mutex g_mtx;

// Fonts
static HFONT g_fontTitle   = nullptr;  // 20px bold
static HFONT g_fontHeading = nullptr;  // 15px bold
static HFONT g_fontNormal  = nullptr;  // 13px medium
static HFONT g_fontBold    = nullptr;  // 13px semibold
static HFONT g_fontSmall   = nullptr;  // 11px regular
static HFONT g_fontSmBold  = nullptr;  // 11px semibold
static HFONT g_fontMono    = nullptr;  // 11px mono
static HFONT g_fontBadge   = nullptr;  // 9px mono bold
static HFONT g_fontTiny    = nullptr;  // 10px

// Brushes
static HBRUSH g_brBG   = nullptr;
static HBRUSH g_brS1   = nullptr;
static HBRUSH g_brS2   = nullptr;

// Navigation
static Page g_currentPage = Page::Crystal;
static int  g_scrollY = 0;
static int  g_contentH = 0;

// Expanded card (one at a time, empty = all collapsed)
static std::string g_expandedCard;

// MC focus state
static bool g_mcFocused = false;

// Running macros
static std::unordered_set<std::string> g_runningMacros;

// Keybind capture
static bool        g_capturing = false;
static std::string g_captureId;     // macro ID
static std::string g_captureField;  // "" = keybind, else slot name

// Optimizer state
static std::unordered_set<std::string> g_appliedOpts;

// ═════════════════════════════════════════════════════════════════════════════
//  NAV ITEMS
// ═════════════════════════════════════════════════════════════════════════════

struct NavItem {
    std::string label;
    Page page;
    bool isSection; // true = section header, false = clickable item
};

static const std::vector<NavItem> g_nav = {
    {"Macros",    Page::Crystal,   true},
    {"Crystal",   Page::Crystal,   false},
    {"Sword",     Page::Sword,     false},
    {"Mace",      Page::Mace,      false},
    {"Cart",      Page::Cart,      false},
    {"UHC",       Page::UHC,       false},
    {"Tools",     Page::Optimizer, true},
    {"Optimizer", Page::Optimizer, false},
    {"Settings",  Page::Settings,  false},
};

// ═════════════════════════════════════════════════════════════════════════════
//  FORWARD DECLARATIONS
// ═════════════════════════════════════════════════════════════════════════════

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Paint
static void paint(HWND hwnd, HDC hdc, RECT cr);
static void paintSidebar(HDC hdc, RECT rc);
static void paintTitlebar(HDC hdc, RECT rc);
static void paintDetBar(HDC hdc, RECT rc);
static void paintChips(HDC hdc, RECT rc);
static void paintMacroPage(HDC hdc, RECT rc, const std::string& category);
static void paintMacroCard(HDC hdc, int x, int y, int w, const config::MacroDef& def, bool expanded);
static void paintSettings(HDC hdc, RECT rc);
static void paintOptimizer(HDC hdc, RECT rc);
static void paintOverlay(HDC hdc, RECT cr);

// Interaction
static void onClick(int x, int y);
static void onKeyCapture(uint16_t vk);
static int  calcPageHeight(const std::string& category);

// ═════════════════════════════════════════════════════════════════════════════
//  HELPERS
// ═════════════════════════════════════════════════════════════════════════════

static HFONT mkFont(int size, int weight = FW_NORMAL, bool mono = false) {
    return CreateFontW(
        -size, 0, 0, 0, weight,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
        mono ? L"Consolas" : L"Segoe UI"
    );
}

static std::wstring toW(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring ws(n - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], n);
    return ws;
}

static void txt(HDC hdc, const std::wstring& s, int x, int y, COLORREF c, HFONT f = nullptr) {
    if (f) SelectObject(hdc, f);
    SetTextColor(hdc, c);
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, x, y, s.c_str(), (int)s.size());
}

static void txtRight(HDC hdc, const std::wstring& s, int x, int y, int maxW, COLORREF c, HFONT f = nullptr) {
    if (f) SelectObject(hdc, f);
    SetTextColor(hdc, c);
    SetBkMode(hdc, TRANSPARENT);
    RECT rc = {x, y, x + maxW, y + 20};
    DrawTextW(hdc, s.c_str(), -1, &rc, DT_RIGHT | DT_SINGLELINE | DT_NOPREFIX);
}

static void fillR(HDC hdc, int x, int y, int w, int h, COLORREF c) {
    RECT rc = {x, y, x + w, y + h};
    HBRUSH br = CreateSolidBrush(c);
    FillRect(hdc, &rc, br);
    DeleteObject(br);
}

static void roundR(HDC hdc, int x, int y, int w, int h, int r, COLORREF fill, COLORREF border) {
    HBRUSH br = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    auto oldBr = SelectObject(hdc, br);
    auto oldPen = SelectObject(hdc, pen);
    RoundRect(hdc, x, y, x + w, y + h, r * 2, r * 2);
    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(br);
    DeleteObject(pen);
}

static void circle(HDC hdc, int cx, int cy, int r, COLORREF c) {
    HBRUSH br = CreateSolidBrush(c);
    HPEN pen = (HPEN)GetStockObject(NULL_PEN);
    auto oldBr = SelectObject(hdc, br);
    auto oldPen = SelectObject(hdc, pen);
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(br);
}

static void drawToggle(HDC hdc, int x, int y, bool on) {
    COLORREF bg = on ? col::TOGGLE_ON : col::TOGGLE_OFF;
    COLORREF border = on ? col::TOGGLE_ON : col::BORDER;
    roundR(hdc, x, y, layout::TOGGLE_W, layout::TOGGLE_H, layout::TOGGLE_R, bg, border);

    // Glow effect when on
    if (on) {
        HPEN glowPen = CreatePen(PS_SOLID, 1, RGB(79, 200, 255));
        auto old = SelectObject(hdc, glowPen);
        auto oldBr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, x - 1, y - 1, x + layout::TOGGLE_W + 1, y + layout::TOGGLE_H + 1,
                  (layout::TOGGLE_R + 1) * 2, (layout::TOGGLE_R + 1) * 2);
        SelectObject(hdc, old);
        SelectObject(hdc, oldBr);
        DeleteObject(glowPen);
    }

    int dotX = on ? (x + layout::TOGGLE_W - layout::TOGGLE_DOT - 3) : (x + 3);
    int dotY = y + (layout::TOGGLE_H - layout::TOGGLE_DOT) / 2;
    HBRUSH dotBr = CreateSolidBrush(on ? RGB(255, 255, 255) : col::T2);
    HPEN np = (HPEN)GetStockObject(NULL_PEN);
    SelectObject(hdc, dotBr);
    SelectObject(hdc, np);
    Ellipse(hdc, dotX, dotY, dotX + layout::TOGGLE_DOT, dotY + layout::TOGGLE_DOT);
    DeleteObject(dotBr);
}

static void hline(HDC hdc, int x, int y, int w, COLORREF c) {
    fillR(hdc, x, y, w, 1, c);
}

static std::string pageToCategory(Page p) {
    switch (p) {
        case Page::Crystal: return "crystal";
        case Page::Sword:   return "sword";
        case Page::Mace:    return "mace";
        case Page::Cart:    return "cart";
        case Page::UHC:     return "uhc";
        default: return "";
    }
}

static std::string pageTitle(Page p) {
    switch (p) {
        case Page::Crystal:   return "Crystal Macros";
        case Page::Sword:     return "Sword Macros";
        case Page::Mace:      return "Mace Macros";
        case Page::Cart:      return "Cart Macros";
        case Page::UHC:       return "UHC Macros";
        case Page::Optimizer: return "Optimizer";
        case Page::Settings:  return "Settings";
        default: return "";
    }
}

static std::string pageSubtitle(Page p) {
    switch (p) {
        case Page::Crystal:   return "Anchor, crystal, and utility macros";
        case Page::Sword:     return "Sword and shield break macros";
        case Page::Mace:      return "Mace and elytra combo macros";
        case Page::Cart:      return "Minecart TNT macros";
        case Page::UHC:       return "Bucket and trap macros";
        case Page::Optimizer: return "OS-level input and system tweaks";
        case Page::Settings:  return "App configuration";
        default: return "";
    }
}

// Card height calculation
static int cardHeight(const config::MacroDef& def, bool expanded) {
    if (!expanded) return layout::CARD_COLLAPSED;
    // Header + body padding + fields
    int nFields = 1 + (int)def.slotNames.size(); // keybind + delay is always shown, + slots
    if (def.id != "fxp") nFields += 1; // delay field (fxp already has it in base)
    return layout::CARD_COLLAPSED + layout::CARD_BODY_PAD * 2 +
           nFields * layout::CARD_FIELD_H;
}

// ═════════════════════════════════════════════════════════════════════════════
//  CREATE
// ═════════════════════════════════════════════════════════════════════════════

HWND create(HINSTANCE hInstance, const Config& cfg, const Callbacks& cbs) {
    g_hInst = hInstance;
    g_cfg = cfg;
    g_cbs = cbs;

    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);

    // Fonts
    g_fontTitle   = mkFont(20, FW_BOLD);
    g_fontHeading = mkFont(15, FW_BOLD);
    g_fontNormal  = mkFont(13, FW_MEDIUM);
    g_fontBold    = mkFont(13, FW_SEMIBOLD);
    g_fontSmall   = mkFont(11, FW_NORMAL);
    g_fontSmBold  = mkFont(11, FW_SEMIBOLD);
    g_fontMono    = mkFont(11, FW_NORMAL, true);
    g_fontBadge   = mkFont(9, FW_BOLD, true);
    g_fontTiny    = mkFont(10, FW_NORMAL);

    // Brushes
    g_brBG = CreateSolidBrush(col::BG);
    g_brS1 = CreateSolidBrush(col::S1);
    g_brS2 = CreateSolidBrush(col::S2);

    // Register
    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance      = hInstance;
    wc.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground  = g_brBG;
    wc.lpszClassName  = L"CrystalSpKMacroWnd";
    RegisterClassExW(&wc);

    int sx = (GetSystemMetrics(SM_CXSCREEN) - layout::WIN_W) / 2;
    int sy = (GetSystemMetrics(SM_CYSCREEN) - layout::WIN_H) / 2;

    g_hwnd = CreateWindowExW(
        0, L"CrystalSpKMacroWnd", L"CrystalSpKMacro",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        sx, sy, layout::WIN_W, layout::WIN_H,
        nullptr, nullptr, hInstance, nullptr
    );

    // Dark title bar
    BOOL dark = TRUE;
    DwmSetWindowAttribute(g_hwnd, 20, &dark, sizeof(dark));

    // Load optimizer state
    for (auto& k : cfg.settings.appliedOpts) g_appliedOpts.insert(k);

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);
    return g_hwnd;
}

void destroy() {
    if (g_hwnd) DestroyWindow(g_hwnd);
    g_hwnd = nullptr;
    // Delete GDI objects
    DeleteObject(g_fontTitle); DeleteObject(g_fontHeading);
    DeleteObject(g_fontNormal); DeleteObject(g_fontBold);
    DeleteObject(g_fontSmall); DeleteObject(g_fontSmBold);
    DeleteObject(g_fontMono); DeleteObject(g_fontBadge);
    DeleteObject(g_fontTiny);
    DeleteObject(g_brBG); DeleteObject(g_brS1); DeleteObject(g_brS2);
}

// ═════════════════════════════════════════════════════════════════════════════
//  WINDOW PROCEDURE
// ═════════════════════════════════════════════════════════════════════════════

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT cr; GetClientRect(hwnd, &cr);

        // Double-buffer
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, cr.right, cr.bottom);
        SelectObject(mem, bmp);

        paint(hwnd, mem, cr);

        BitBlt(hdc, 0, 0, cr.right, cr.bottom, mem, 0, 0, SRCCOPY);
        DeleteObject(bmp);
        DeleteDC(mem);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_LBUTTONDOWN:
        onClick(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_KEYDOWN:
        if (g_capturing) {
            onKeyCapture(static_cast<uint16_t>(wParam));
            return 0;
        }
        break;

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        g_scrollY -= delta / 3;
        g_scrollY = std::max(0, std::min(g_scrollY, std::max(0, g_contentH - 400)));
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    }

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_TRAYICON:
        break; // forwarded externally
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ═════════════════════════════════════════════════════════════════════════════
//  MAIN PAINT
// ═════════════════════════════════════════════════════════════════════════════

static void paint(HWND hwnd, HDC hdc, RECT cr) {
    // Full background
    FillRect(hdc, &cr, g_brBG);

    // Sidebar
    RECT sideRc = {0, 0, layout::SIDEBAR_W, cr.bottom};
    paintSidebar(hdc, sideRc);

    // Separator
    fillR(hdc, layout::SIDEBAR_W, 0, 1, cr.bottom, col::BORDER);

    // Content area
    int cx = layout::SIDEBAR_W + 1;
    int cw = cr.right - cx;

    // Titlebar
    RECT tbRc = {cx, 0, cr.right, layout::TITLEBAR_H};
    paintTitlebar(hdc, tbRc);
    hline(hdc, cx, layout::TITLEBAR_H, cw, col::BORDER);

    // Detection bar
    int detY = layout::TITLEBAR_H + 1;
    RECT detRc = {cx, detY, cr.right, detY + layout::DET_BAR_H};
    paintDetBar(hdc, detRc);
    hline(hdc, cx, detY + layout::DET_BAR_H, cw, col::BORDER);

    // Chips
    int chipY = detY + layout::DET_BAR_H + 1;
    RECT chipRc = {cx, chipY, cr.right, chipY + layout::CHIPS_H};
    paintChips(hdc, chipRc);

    // Main content (clipped + scrolled)
    int contentY = chipY + layout::CHIPS_H;
    RECT contentRc = {cx, contentY, cr.right, cr.bottom};

    // Set clip region
    HRGN clipRgn = CreateRectRgn(contentRc.left, contentRc.top, contentRc.right, contentRc.bottom);
    SelectClipRgn(hdc, clipRgn);

    // Apply scroll offset
    int offsetY = contentY - g_scrollY;

    std::string cat = pageToCategory(g_currentPage);
    if (!cat.empty()) {
        RECT pgRc = {cx, offsetY, cr.right, offsetY + 5000};
        paintMacroPage(hdc, pgRc, cat);
    } else if (g_currentPage == Page::Settings) {
        RECT pgRc = {cx, offsetY, cr.right, offsetY + 5000};
        paintSettings(hdc, pgRc);
    } else if (g_currentPage == Page::Optimizer) {
        RECT pgRc = {cx, offsetY, cr.right, offsetY + 5000};
        paintOptimizer(hdc, pgRc);
    }

    // Remove clip
    SelectClipRgn(hdc, nullptr);
    DeleteObject(clipRgn);

    // Overlay (keybind capture)
    if (g_capturing) {
        paintOverlay(hdc, cr);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  SIDEBAR
// ═════════════════════════════════════════════════════════════════════════════

static void paintSidebar(HDC hdc, RECT rc) {
    // Gradient-ish background: slightly lighter at top
    fillR(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, col::SIDEBAR_BG);

    // Subtle top accent gradient (fake it with a few rects)
    for (int i = 0; i < 40; i++) {
        int alpha = 40 - i;
        // Blend accent into sidebar bg
        int r = (14 * (255 - alpha) + 79 * alpha) / 255;
        int g = (18 * (255 - alpha) + 200 * alpha) / 255;
        int b = (28 * (255 - alpha) + 255 * alpha) / 255;
        fillR(hdc, rc.left, rc.top + i * 2, rc.right - rc.left, 2,
              RGB(std::min(r, 35), std::min(g, 38), std::min(b, 60)));
    }

    // App branding
    txt(hdc, L"CrystalSpK", rc.left + 18, 16, col::ACCENT, g_fontHeading);
    txt(hdc, L"Macro", rc.left + 130, 18, col::T2, g_fontSmall);
    txt(hdc, L"v1.2.0", rc.left + 18, 36, col::T3, g_fontTiny);

    // Nav items
    int y = 60;
    for (auto& item : g_nav) {
        if (item.isSection) {
            // Section header
            y += 6;
            txt(hdc, toW(item.label), rc.left + 16, y + 6, col::T2, g_fontBadge);
            y += layout::NAV_SECTION_H;
            continue;
        }

        bool active = (item.page == g_currentPage);

        if (active) {
            // Active background with subtle accent
            roundR(hdc, rc.left + 6, y, rc.right - rc.left - 12, layout::NAV_ITEM_H,
                   6, col::NAV_ACTIVE_BG, RGB(30, 45, 75));
            // Left accent bar
            fillR(hdc, rc.left + 6, y + 6, 3, layout::NAV_ITEM_H - 12, col::NAV_ACTIVE_BAR);
        }

        COLORREF textCol = active ? col::T0 : col::T1;
        HFONT font = active ? g_fontBold : g_fontNormal;

        // Count active macros in this category
        std::string cat = pageToCategory(item.page);
        int activeCount = 0;
        if (!cat.empty()) {
            std::lock_guard<std::mutex> lk(g_mtx);
            for (auto& [id, mc] : g_cfg.macros) {
                if (mc.active) {
                    for (auto& def : config::getAllMacroDefs()) {
                        if (def.id == id && def.category == cat) { activeCount++; break; }
                    }
                }
            }
        }

        txt(hdc, toW(item.label), rc.left + 24, y + 9, textCol, font);

        // Active count badge
        if (activeCount > 0) {
            std::wstring badge = std::to_wstring(activeCount);
            txtRight(hdc, badge, rc.left + 24, y + 10,
                     rc.right - rc.left - 48, col::ACCENT, g_fontMono);
        }

        y += layout::NAV_ITEM_H + 2;
    }

    // Footer
    txt(hdc, L"native build", rc.left + 18, rc.bottom - 26, col::T3, g_fontTiny);
}

// ═════════════════════════════════════════════════════════════════════════════
//  TITLEBAR
// ═════════════════════════════════════════════════════════════════════════════

static void paintTitlebar(HDC hdc, RECT rc) {
    // Subtle gradient bg
    fillR(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, col::S1);

    // Page title
    txt(hdc, toW(pageTitle(g_currentPage)), rc.left + 20, 12, col::T0, g_fontBold);

    // Subtitle
    txt(hdc, toW(pageSubtitle(g_currentPage)), rc.left + 200, 15, col::T2, g_fontSmall);
}

// ═════════════════════════════════════════════════════════════════════════════
//  DETECTION BAR
// ═════════════════════════════════════════════════════════════════════════════

static void paintDetBar(HDC hdc, RECT rc) {
    fillR(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, col::DET_BG);

    // LED dot
    int dotX = rc.left + 18;
    int dotY = rc.top + (layout::DET_BAR_H - 7) / 2;
    COLORREF dotColor = g_mcFocused ? col::GREEN : col::T3;
    circle(hdc, dotX + 3, dotY + 3, 4, dotColor);

    // Glow if focused
    if (g_mcFocused) {
        circle(hdc, dotX + 3, dotY + 3, 6, RGB(30, 60, 80));
        circle(hdc, dotX + 3, dotY + 3, 4, dotColor);
    }

    // Status text
    std::wstring status = g_mcFocused ? L"Minecraft detected — macros active" :
                                        L"Minecraft not focused — macros paused";
    COLORREF statusCol = g_mcFocused ? col::GREEN : col::T2;
    txt(hdc, status, rc.left + 34, rc.top + 11, statusCol, g_fontSmall);

    // Focus lock indicator
    bool fl = g_cfg.settings.focusLock;
    std::wstring flText = fl ? L"Focus Lock: ON" : L"Focus Lock: OFF";
    txtRight(hdc, flText, rc.left, rc.top + 11, rc.right - rc.left - 16,
             fl ? col::ACCENT : col::T2, g_fontSmBold);
}

// ═════════════════════════════════════════════════════════════════════════════
//  ACTIVE CHIPS
// ═════════════════════════════════════════════════════════════════════════════

static void paintChips(HDC hdc, RECT rc) {
    int x = rc.left + layout::CARD_PAD_X;
    int y = rc.top + 6;

    std::lock_guard<std::mutex> lk(g_mtx);
    bool any = false;

    for (auto& id : g_runningMacros) {
        any = true;
        std::string upper = id;
        for (auto& c : upper) c = static_cast<char>(toupper(c));

        int chipW = 10 + (int)upper.size() * 7 + 18;
        roundR(hdc, x, y, chipW, 20, 10, col::CHIP_BG, col::BORDER);
        circle(hdc, x + 10, y + 10, 3, col::CHIP_DOT);
        txt(hdc, toW(upper), x + 18, y + 3, col::T0, g_fontBadge);
        x += chipW + 6;
    }

    // Also show active (enabled) macros
    for (auto& [id, mc] : g_cfg.macros) {
        if (!mc.active) continue;
        if (g_runningMacros.count(id)) continue; // already shown
        any = true;

        std::string upper = id;
        for (auto& c : upper) c = static_cast<char>(toupper(c));

        int chipW = 10 + (int)upper.size() * 7 + 18;
        roundR(hdc, x, y, chipW, 20, 10, RGB(15, 18, 28), col::BORDER);
        circle(hdc, x + 10, y + 10, 3, col::T3);
        txt(hdc, toW(upper), x + 18, y + 3, col::T2, g_fontBadge);
        x += chipW + 6;

        if (x > rc.right - 40) break; // overflow
    }

    if (!any) {
        txt(hdc, L"No active macros — enable one to get started", rc.left + layout::CARD_PAD_X,
            y + 2, col::T2, g_fontSmall);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  MACRO PAGE (2-column grid)
// ═════════════════════════════════════════════════════════════════════════════

static void paintMacroPage(HDC hdc, RECT rc, const std::string& category) {
    const auto& defs = config::getAllMacroDefs();
    int px = rc.left + layout::CARD_PAD_X;
    int cw = (rc.right - rc.left - layout::CARD_PAD_X * 2 - layout::CARD_GAP) / 2;

    // Page header
    int y = rc.top + 12;

    // 2-column layout
    int col0X = px;
    int col1X = px + cw + layout::CARD_GAP;
    int col0Y = y;
    int col1Y = y;
    int colIdx = 0;

    for (auto& def : defs) {
        if (def.category != category) continue;

        bool expanded = (g_expandedCard == def.id);
        int h = cardHeight(def, expanded);

        int cx = (colIdx % 2 == 0) ? col0X : col1X;
        int& cy = (colIdx % 2 == 0) ? col0Y : col1Y;

        paintMacroCard(hdc, cx, cy, cw, def, expanded);
        cy += h + layout::CARD_GAP;
        colIdx++;
    }

    g_contentH = std::max(col0Y, col1Y) - rc.top + 40;
}

// ═════════════════════════════════════════════════════════════════════════════
//  MACRO CARD
// ═════════════════════════════════════════════════════════════════════════════

static void paintMacroCard(HDC hdc, int x, int y, int w, const config::MacroDef& def, bool expanded) {
    std::lock_guard<std::mutex> lk(g_mtx);

    auto it = g_cfg.macros.find(def.id);
    bool active = (it != g_cfg.macros.end()) ? it->second.active : false;
    bool running = g_runningMacros.count(def.id) > 0;
    std::string keybind = (it != g_cfg.macros.end()) ? it->second.keybind : "None";
    int delay = (it != g_cfg.macros.end()) ? it->second.delay : def.defaultDelay;

    int h = cardHeight(def, expanded);

    // Card background
    COLORREF cardBg = active ? col::CARD_ACTIVE : col::S1;
    COLORREF cardBorder = active ? RGB(30, 50, 80) : col::BORDER;

    roundR(hdc, x, y, w, h, layout::RADIUS_LG, cardBg, cardBorder);

    // Active glow effect
    if (active) {
        // Top accent line (like Electron .sblock::before)
        for (int i = 0; i < w / 2; i++) {
            int alpha = 255 - (i * 255 / (w / 2));
            int r2 = (79 * alpha) / 255;
            int g2 = (200 * alpha) / 255;
            int b2 = (255 * alpha) / 255;
            if (r2 < 15) r2 = 15;
            fillR(hdc, x + layout::RADIUS_LG + i, y + 1, 1, 1,
                  RGB(std::min(r2, 80), std::min(g2, 200), std::min(b2, 255)));
        }
    }

    // ── Header section ──────────────────────────────────────────────────────

    // Badge (macro ID)
    std::string badge = def.id;
    for (auto& c : badge) c = static_cast<char>(toupper(c));
    COLORREF badgeBg = active ? RGB(20, 35, 55) : col::S3;
    COLORREF badgeText = active ? col::ACCENT : col::T1;
    COLORREF badgeBorder = active ? RGB(30, 55, 90) : col::BORDER;
    roundR(hdc, x + 14, y + 14, 32, 20, 5, badgeBg, badgeBorder);
    txt(hdc, toW(badge), x + 18, y + 16, badgeText, g_fontBadge);

    // Name
    txt(hdc, toW(def.name), x + 54, y + 13, col::T0, g_fontBold);

    // Keybind tag
    std::string kbDisplay = "[" + keybind + "]";
    txt(hdc, toW(kbDisplay), x + 54, y + 32, col::T2, g_fontMono);

    // Running status dot
    if (running) {
        circle(hdc, x + w - 56, y + 24, 4, col::GREEN);
    }

    // Toggle
    int togX = x + w - layout::TOGGLE_W - 12;
    int togY = y + (layout::CARD_COLLAPSED - layout::TOGGLE_H) / 2;
    drawToggle(hdc, togX, togY, active);

    // ── Body (expanded) ─────────────────────────────────────────────────────

    if (expanded) {
        int bodyY = y + layout::CARD_COLLAPSED;
        hline(hdc, x + 12, bodyY - 1, w - 24, col::BORDER);

        int fy = bodyY + layout::CARD_BODY_PAD;

        // Keybind field
        txt(hdc, L"Keybind", x + 16, fy + 6, col::T1, g_fontSmall);
        roundR(hdc, x + w - layout::FIELD_INPUT_W - 16, fy + 2,
               layout::FIELD_INPUT_W, layout::FIELD_INPUT_H, 6, col::S2, col::BORDER);
        txt(hdc, toW(keybind), x + w - layout::FIELD_INPUT_W - 6, fy + 7,
            col::T0, g_fontMono);
        fy += layout::CARD_FIELD_H;

        // Delay field
        txt(hdc, L"Delay (ms)", x + 16, fy + 6, col::T1, g_fontSmall);
        roundR(hdc, x + w - layout::FIELD_INPUT_W - 16, fy + 2,
               layout::FIELD_INPUT_W, layout::FIELD_INPUT_H, 6, col::S2, col::BORDER);
        txt(hdc, toW(std::to_string(delay)), x + w - layout::FIELD_INPUT_W - 6, fy + 7,
            col::T0, g_fontMono);
        fy += layout::CARD_FIELD_H;

        // Slot fields
        for (auto& slotName : def.slotNames) {
            // Pretty label
            std::string label = slotName;
            // Convert camelCase to display: "anchorKey" → "Anchor key"
            if (label.size() > 3 && label.substr(label.size() - 3) == "Key") {
                label = label.substr(0, label.size() - 3) + " key";
            }
            if (!label.empty()) label[0] = static_cast<char>(toupper(label[0]));

            std::string val = "None";
            if (it != g_cfg.macros.end()) {
                auto sit = it->second.slots.find(slotName);
                if (sit != it->second.slots.end()) val = sit->second;
            }

            txt(hdc, toW(label), x + 16, fy + 6, col::T1, g_fontSmall);
            roundR(hdc, x + w - layout::FIELD_INPUT_W - 16, fy + 2,
                   layout::FIELD_INPUT_W, layout::FIELD_INPUT_H, 6, col::S2, col::BORDER);
            txt(hdc, toW(val), x + w - layout::FIELD_INPUT_W - 6, fy + 7,
                col::T0, g_fontMono);
            fy += layout::CARD_FIELD_H;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  SETTINGS PAGE
// ═════════════════════════════════════════════════════════════════════════════

static void paintSettings(HDC hdc, RECT rc) {
    int x = rc.left + layout::CARD_PAD_X;
    int w = rc.right - rc.left - layout::CARD_PAD_X * 2;
    int y = rc.top + 16;

    // Section: General
    txt(hdc, L"GENERAL", x, y, col::T2, g_fontBadge);
    hline(hdc, x + 70, y + 5, w - 70, col::BORDER);
    y += 24;

    // Card-style settings block
    roundR(hdc, x, y, w, 180, layout::RADIUS_LG, col::S1, col::BORDER);

    // Focus Lock
    int ry = y + 16;
    txt(hdc, L"Focus Lock", x + 16, ry, col::T0, g_fontBold);
    txt(hdc, L"Only fire macros when Minecraft is focused", x + 16, ry + 20, col::T1, g_fontSmall);
    drawToggle(hdc, x + w - layout::TOGGLE_W - 16, ry + 4, g_cfg.settings.focusLock);
    ry += 50;

    hline(hdc, x + 12, ry, w - 24, col::BORDER);
    ry += 12;

    // Chat Key
    txt(hdc, L"Chat Key", x + 16, ry, col::T0, g_fontBold);
    txt(hdc, L"Key you press to open Minecraft chat", x + 16, ry + 20, col::T1, g_fontSmall);
    roundR(hdc, x + w - 80, ry + 2, 64, 26, 6, col::S2, col::BORDER);
    txt(hdc, toW(g_cfg.settings.chatKey), x + w - 70, ry + 7, col::ACCENT, g_fontMono);
    ry += 50;

    hline(hdc, x + 12, ry, w - 24, col::BORDER);
    ry += 12;

    // Chat Timer
    txt(hdc, L"Failsafe Timer", x + 16, ry, col::T0, g_fontBold);
    txt(hdc, L"Auto-resume macros after chat", x + 16, ry + 20, col::T1, g_fontSmall);
    roundR(hdc, x + w - 80, ry + 2, 64, 26, 6, col::S2, col::BORDER);
    txt(hdc, toW(std::to_string(g_cfg.settings.chatTimer) + "s"), x + w - 70, ry + 7,
        col::ACCENT, g_fontMono);

    // Emergency stop button
    y += 220;
    txt(hdc, L"EMERGENCY", x, y, col::T2, g_fontBadge);
    hline(hdc, x + 80, y + 5, w - 80, col::BORDER);
    y += 24;

    roundR(hdc, x, y, 140, 38, 8, RGB(40, 15, 20), RGB(240, 94, 94));
    txt(hdc, L"Stop All Macros", x + 16, y + 10, col::RED, g_fontBold);

    g_contentH = y - rc.top + 80;
}

// ═════════════════════════════════════════════════════════════════════════════
//  OPTIMIZER PAGE
// ═════════════════════════════════════════════════════════════════════════════

static void paintOptimizer(HDC hdc, RECT rc) {
    int x = rc.left + layout::CARD_PAD_X;
    int w = rc.right - rc.left - layout::CARD_PAD_X * 2;
    int y = rc.top + 12;

    // Hero card
    roundR(hdc, x, y, w, 70, layout::RADIUS_LG, col::S1, col::BORDER);
    txt(hdc, L"Input & System Optimizer", x + 16, y + 12, col::T0, g_fontHeading);
    txt(hdc, L"Low-level Windows tweaks to cut input lag and tighten macro timing",
        x + 16, y + 34, col::T1, g_fontSmall);

    // Stats
    int applied = (int)g_appliedOpts.size();
    std::wstring stats = std::to_wstring(applied) + L" / 12 active";
    txtRight(hdc, stats, x, y + 14, w - 16, col::ACCENT, g_fontSmBold);

    y += 86;

    struct OptRow {
        std::string key;
        std::wstring name;
        std::wstring desc;
        bool high;
    };

    struct OptCat {
        std::wstring label;
        std::vector<OptRow> rows;
    };

    std::vector<OptCat> cats = {
        {L"KEYBOARD", {
            {"keyrepeat",  L"Key Repeat Delay & Rate", L"Sets delay to minimum, rate to maximum", true},
            {"stickykeys", L"Disable Sticky & Filter Keys", L"Prevents 5x-Shift popup during macro sequences", true},
        }},
        {L"MOUSE", {
            {"accel",    L"Disable Mouse Acceleration", L"True 1:1 raw movement, consistent aim", true},
            {"rawinput", L"Raw Input Enforcement", L"Bypasses Windows mouse smoothing entirely", true},
        }},
        {L"SYSTEM", {
            {"priority",   L"MC Process Priority → High", L"Elevates CPU priority, reduces input lag", true},
            {"fullscreen", L"Disable Fullscreen Optimizations", L"Removes hidden 1-3 frame latency", false},
            {"timer",      L"Timer Resolution → 1ms", L"Precise macro delays instead of 15ms OS windows", true},
            {"network",    L"TCP Network Tuning", L"TcpAckFrequency + NoDelay for lower ping", false},
        }},
        {L"FPS BOOST", {
            {"gamedvr",   L"Disable Game Bar & DVR", L"Frees 5-15% GPU overhead from background capture", true},
            {"powerplan", L"High Performance Power Plan", L"Max CPU clock, no throttling or core parking", true},
            {"visualfx",  L"Disable Visual Effects", L"Turns off animations, transparency, shadows", false},
            {"gpusched",  L"GPU Hardware Scheduling", L"HAGS — GPU manages VRAM directly (needs reboot)", false},
        }},
    };

    for (auto& cat : cats) {
        txt(hdc, cat.label, x, y, col::T2, g_fontBadge);
        hline(hdc, x + 90, y + 5, w - 90, col::BORDER);
        y += 20;

        for (auto& row : cat.rows) {
            bool on = g_appliedOpts.count(row.key) > 0;

            COLORREF rowBg = on ? RGB(15, 22, 38) : col::S1;
            COLORREF rowBorder = on ? RGB(28, 48, 80) : col::BORDER;
            roundR(hdc, x, y, w, 50, layout::RADIUS, rowBg, rowBorder);

            // Impact badge
            COLORREF impactBg = row.high ? RGB(15, 28, 42) : RGB(30, 25, 15);
            COLORREF impactText = row.high ? col::GREEN : col::YELLOW;
            roundR(hdc, x + 12, y + 16, 38, 18, 4, impactBg, impactBg);
            txt(hdc, row.high ? L"HIGH" : L"MED", x + 17, y + 18, impactText, g_fontBadge);

            // Name + desc
            txt(hdc, row.name, x + 58, y + 10, col::T0, g_fontBold);
            txt(hdc, row.desc, x + 58, y + 28, col::T1, g_fontSmall);

            // Toggle
            drawToggle(hdc, x + w - layout::TOGGLE_W - 12, y + 16, on);

            y += 54;
        }
        y += 8;
    }

    g_contentH = y - rc.top + 40;
}

// ═════════════════════════════════════════════════════════════════════════════
//  KEYBIND CAPTURE OVERLAY
// ═════════════════════════════════════════════════════════════════════════════

static void paintOverlay(HDC hdc, RECT cr) {
    // Semi-transparent dark overlay (fake it with a solid dark rect)
    fillR(hdc, 0, 0, cr.right, cr.bottom, col::OVERLAY);

    // Center prompt card
    int cardW = 320;
    int cardH = 140;
    int cx = (cr.right - cardW) / 2;
    int cy = (cr.bottom - cardH) / 2;

    roundR(hdc, cx, cy, cardW, cardH, 16, col::S1, col::ACCENT);

    // Accent top line
    fillR(hdc, cx + 16, cy + 1, cardW - 32, 2, col::ACCENT);

    txt(hdc, L"Press any key", cx + 80, cy + 30, col::T0, g_fontTitle);

    std::string target = g_captureField.empty() ? "keybind" : g_captureField;
    std::wstring sub = L"Binding: " + toW(g_captureId) + L" → " + toW(target);
    txt(hdc, sub, cx + 60, cy + 65, col::T1, g_fontSmall);

    txt(hdc, L"Press ESC to cancel", cx + 90, cy + 100, col::T2, g_fontSmall);
}

// ═════════════════════════════════════════════════════════════════════════════
//  CLICK HANDLING
// ═════════════════════════════════════════════════════════════════════════════

static void onClick(int x, int y) {
    // Overlay — click anywhere dismisses (ESC also works)
    if (g_capturing) {
        g_capturing = false;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }

    // ── Sidebar navigation ──────────────────────────────────────────────────
    if (x < layout::SIDEBAR_W) {
        int ny = 60;
        for (auto& item : g_nav) {
            if (item.isSection) {
                ny += 6 + layout::NAV_SECTION_H;
                continue;
            }
            if (y >= ny && y < ny + layout::NAV_ITEM_H) {
                if (g_currentPage != item.page) {
                    g_currentPage = item.page;
                    g_scrollY = 0;
                    g_expandedCard.clear();
                    InvalidateRect(g_hwnd, nullptr, FALSE);
                }
                return;
            }
            ny += layout::NAV_ITEM_H + 2;
        }
        return;
    }

    // ── Content area ────────────────────────────────────────────────────────
    int cx = layout::SIDEBAR_W + 1;
    int contentTop = layout::TITLEBAR_H + 1 + layout::DET_BAR_H + 1 + layout::CHIPS_H;

    // Detection bar — focus lock toggle click
    int detTop = layout::TITLEBAR_H + 1;
    if (y >= detTop && y < detTop + layout::DET_BAR_H && x > layout::WIN_W - 150) {
        g_cfg.settings.focusLock = !g_cfg.settings.focusLock;
        if (g_cbs.onFocusLockChanged) g_cbs.onFocusLockChanged(g_cfg.settings.focusLock);
        if (g_cbs.onConfigChanged) g_cbs.onConfigChanged(g_cfg);
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }

    // Macro page clicks
    std::string cat = pageToCategory(g_currentPage);
    if (!cat.empty() && y >= contentTop) {
        int adjustedY = y + g_scrollY - contentTop;

        const auto& defs = config::getAllMacroDefs();
        int cw = (layout::WIN_W - cx - layout::CARD_PAD_X * 2 - layout::CARD_GAP) / 2;
        int col0X = cx + layout::CARD_PAD_X;
        int col1X = col0X + cw + layout::CARD_GAP;

        int col0Y = 12, col1Y = 12;
        int colIdx = 0;

        for (auto& def : defs) {
            if (def.category != cat) continue;

            bool expanded = (g_expandedCard == def.id);
            int h = cardHeight(def, expanded);
            int cardX = (colIdx % 2 == 0) ? col0X : col1X;
            int& cardY = (colIdx % 2 == 0) ? col0Y : col1Y;

            if (adjustedY >= cardY && adjustedY < cardY + h &&
                x >= cardX && x < cardX + cw) {

                // Toggle button hit test
                int togX = cardX + cw - layout::TOGGLE_W - 12;
                int togY = cardY + (layout::CARD_COLLAPSED - layout::TOGGLE_H) / 2;
                if (x >= togX && x < togX + layout::TOGGLE_W &&
                    adjustedY >= togY && adjustedY < togY + layout::TOGGLE_H) {
                    std::lock_guard<std::mutex> lk(g_mtx);
                    g_cfg.macros[def.id].active = !g_cfg.macros[def.id].active;
                    if (g_cbs.onConfigChanged) g_cbs.onConfigChanged(g_cfg);
                    InvalidateRect(g_hwnd, nullptr, FALSE);
                    return;
                }

                // Field click (if expanded) — start keybind capture
                if (expanded && adjustedY >= cardY + layout::CARD_COLLAPSED) {
                    int fieldIdx = (adjustedY - cardY - layout::CARD_COLLAPSED - layout::CARD_BODY_PAD) / layout::CARD_FIELD_H;
                    int fieldX = cardX + cw - layout::FIELD_INPUT_W - 16;

                    if (x >= fieldX && x < fieldX + layout::FIELD_INPUT_W) {
                        g_capturing = true;
                        g_captureId = def.id;
                        if (fieldIdx == 0) g_captureField = "";          // keybind
                        else if (fieldIdx == 1) g_captureField = "__delay"; // delay (handled differently)
                        else if (fieldIdx - 2 < (int)def.slotNames.size()) {
                            g_captureField = def.slotNames[fieldIdx - 2];
                        }
                        InvalidateRect(g_hwnd, nullptr, FALSE);
                        return;
                    }
                }

                // Click on card header — expand/collapse
                if (adjustedY < cardY + layout::CARD_COLLAPSED) {
                    g_expandedCard = expanded ? "" : def.id;
                    InvalidateRect(g_hwnd, nullptr, FALSE);
                    return;
                }
                return;
            }

            cardY += h + layout::CARD_GAP;
            colIdx++;
        }
    }

    // Settings page clicks
    if (g_currentPage == Page::Settings) {
        int adjustedY = y + g_scrollY - contentTop;

        // Focus lock toggle
        int settingsW = layout::WIN_W - cx - layout::CARD_PAD_X * 2;
        int togX = cx + layout::CARD_PAD_X + settingsW - layout::TOGGLE_W - 16;
        if (adjustedY >= 36 && adjustedY < 56 && x >= togX) {
            g_cfg.settings.focusLock = !g_cfg.settings.focusLock;
            if (g_cbs.onFocusLockChanged) g_cbs.onFocusLockChanged(g_cfg.settings.focusLock);
            if (g_cbs.onConfigChanged) g_cbs.onConfigChanged(g_cfg);
            InvalidateRect(g_hwnd, nullptr, FALSE);
            return;
        }

        // Stop All button
        if (adjustedY >= 260 && adjustedY < 298 && x >= cx + layout::CARD_PAD_X && x < cx + layout::CARD_PAD_X + 140) {
            if (g_cbs.onStopAll) g_cbs.onStopAll();
            return;
        }
    }

    // Optimizer page — toggle clicks
    if (g_currentPage == Page::Optimizer) {
        int adjustedY = y + g_scrollY - contentTop;

        // Calculate positions matching paintOptimizer
        int ox = cx + layout::CARD_PAD_X;
        int ow = layout::WIN_W - cx - layout::CARD_PAD_X * 2;
        int oy = 12 + 86; // after hero

        const auto& allKeys = optimizer::allKeys();
        // Category structure matching paintOptimizer
        int catSizes[] = {2, 2, 4, 4};
        int catIdx = 0, rowInCat = 0, keyIdx = 0;

        for (int i = 0; i < 4; i++) {
            oy += 20; // category label
            for (int j = 0; j < catSizes[i]; j++) {
                if (keyIdx < (int)allKeys.size()) {
                    int togX2 = ox + ow - layout::TOGGLE_W - 12;
                    if (adjustedY >= oy + 16 && adjustedY < oy + 34 &&
                        x >= togX2 && x < togX2 + layout::TOGGLE_W) {
                        const std::string& key = allKeys[keyIdx];
                        if (g_appliedOpts.count(key)) {
                            g_appliedOpts.erase(key);
                            if (g_cbs.onRevertOpt) g_cbs.onRevertOpt(key);
                        } else {
                            g_appliedOpts.insert(key);
                            if (g_cbs.onApplyOpt) g_cbs.onApplyOpt(key);
                        }
                        InvalidateRect(g_hwnd, nullptr, FALSE);
                        return;
                    }
                    keyIdx++;
                }
                oy += 54;
            }
            oy += 8;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  KEYBIND CAPTURE
// ═════════════════════════════════════════════════════════════════════════════

static void onKeyCapture(uint16_t vk) {
    if (!g_capturing) return;

    if (vk == VK_ESCAPE) {
        g_capturing = false;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }

    std::string keyName = vkToName(vk);
    if (keyName.empty()) return;

    {
        std::lock_guard<std::mutex> lk(g_mtx);
        if (g_captureField.empty()) {
            // Main keybind
            g_cfg.macros[g_captureId].keybind = keyName;
        } else if (g_captureField == "__delay") {
            // Delay — just set a numeric value based on key
            if (vk >= '0' && vk <= '9') {
                int digit = vk - '0';
                g_cfg.macros[g_captureId].delay = digit * 10;
            }
        } else {
            // Slot key
            g_cfg.macros[g_captureId].slots[g_captureField] = keyName;
        }
    }

    g_capturing = false;
    if (g_cbs.onConfigChanged) g_cbs.onConfigChanged(g_cfg);
    InvalidateRect(g_hwnd, nullptr, FALSE);
}

// ═════════════════════════════════════════════════════════════════════════════
//  PUBLIC API
// ═════════════════════════════════════════════════════════════════════════════

void setMacroRunning(const std::string& id, bool running) {
    std::lock_guard<std::mutex> lk(g_mtx);
    if (running) g_runningMacros.insert(id);
    else g_runningMacros.erase(id);
    if (g_hwnd) PostMessage(g_hwnd, WM_USER + 10, 0, 0); // trigger repaint from any thread
}

void setMcFocused(bool focused) {
    g_mcFocused = focused;
    if (g_hwnd) InvalidateRect(g_hwnd, nullptr, FALSE);
}

void reloadConfig(const Config& cfg) {
    std::lock_guard<std::mutex> lk(g_mtx);
    g_cfg = cfg;
    if (g_hwnd) InvalidateRect(g_hwnd, nullptr, FALSE);
}

Config getCurrentConfig() {
    std::lock_guard<std::mutex> lk(g_mtx);
    return g_cfg;
}

void show() {
    if (g_hwnd) { ShowWindow(g_hwnd, SW_SHOW); SetForegroundWindow(g_hwnd); }
}

void hide() {
    if (g_hwnd) ShowWindow(g_hwnd, SW_HIDE);
}

bool isVisible() {
    return g_hwnd && IsWindowVisible(g_hwnd);
}

void invalidate() {
    if (g_hwnd) InvalidateRect(g_hwnd, nullptr, FALSE);
}

} // namespace gui
#endif // _WIN32
