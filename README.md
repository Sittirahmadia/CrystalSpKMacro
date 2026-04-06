# CrystalSpKMacro — Native C++ Build

Standalone Windows .exe — no Electron, no Node.js, no runtime dependencies.

## Size Comparison

| Version | Size |
|---------|------|
| Electron (.exe installer) | ~200MB |
| **Native C++ (.exe)** | **~5-10MB** |

## Features

All 21 macros, fully native Win32 SendInput:

### Crystal Macros
- **SA** — Single Anchor (concurrent slot+click)
- **DA** — Double Anchor (airplace technique)
- **AP** — Anchor Pearl
- **HC** — Hit Crystal (obsi + crystal + detonate)
- **SHC** — Slow Hit Crystal (high-latency servers)
- **AC** — Auto Crystal (hold-to-run loop)
- **KP** — Key Pearl
- **IDH** — Inventory D-Hand
- **OHT** — Offhand Totem
- **FXP** — Fast XP (hold-to-run)

### Sword Macros
- **ASB** — Auto Shield Breaker
- **LS** — Lunge Swap (one-tick attribute swap)

### Mace Macros
- **ES** — Elytra Swap
- **PC** — Pearl Catch
- **SS** — Stun Slam
- **BS** — Breach Swap

### Cart Macros
- **IC** — Insta Cart
- **XB** — Crossbow Cart

### UHC Macros
- **DR** — Drain
- **LW** — Lava Web
- **LA** — Lava

### System Optimizer (12 tweaks)
- Key Repeat, Sticky Keys, Mouse Accel, Raw Input
- MC Priority, Fullscreen Opt, Timer Resolution, Network
- Game DVR, Power Plan, Visual FX, GPU Scheduling

## Building

### Prerequisites

- **Windows 10/11**
- **Visual Studio 2022** (or Build Tools) with "Desktop development with C++" workload
- **CMake 3.20+** (included with Visual Studio)

### Quick Build

```batch
cd native-app
build.bat
```

Output: `build/Release/CrystalSpKMacro.exe`

### Manual Build

```batch
cd native-app
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release --parallel
```

### Debug Build

```batch
build.bat debug
```

### Clean

```batch
build.bat clean
```

## Architecture

```
src/
├── main.cpp        — Entry point, message loop, timers
├── input.h/cpp     — SendInput primitives, timing, key mapping
├── macros.h/cpp    — All 21 macro implementations
├── engine.h/cpp    — WH_KEYBOARD_LL / WH_MOUSE_LL hook engine
├── config.h/cpp    — JSON config load/save
├── gui.h/cpp       — Win32 dark-themed GUI (owner-drawn)
├── tray.h/cpp      — System tray icon + menu
├── optimizer.h/cpp — Registry-based Windows optimizations
└── resource.h      — Resource IDs

vendor/
└── nlohmann/json.hpp  — JSON library (header-only)

resources/
└── app.rc          — Version info + icon resources
```

## Config

Config is saved at `%APPDATA%/CrystalSpKMacro/config.json` — same format as the Electron version, so you can copy your existing config over.

## Technical Details

- **Input method:** Win32 `SendInput` with `KEYEVENTF_SCANCODE` + hardware mouse flags
- **Timing:** `timeBeginPeriod(1)` + high-resolution spin-wait for sub-ms precision
- **Concurrency:** Batched `SendInput` arrays for concurrent key+click (zero gap)
- **Hooks:** `WH_KEYBOARD_LL` / `WH_MOUSE_LL` with `LLKHF_INJECTED` filtering
- **GUI:** GDI owner-drawn with double-buffering, DWM dark title bar
- **Static linking:** MSVC `/MT` — single .exe with zero DLL dependencies
