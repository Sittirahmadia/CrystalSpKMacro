# noqwdmacro v1.4.0

A high-performance, low-latency competitive gaming macro application for Minecraft Crystal PVP, Mace, and Sword techniques. Built in C++17 with an ImGui + DirectX 11 GUI.

---

## Features

### Crystal PVP Automation
| Macro | Description |
|-------|-------------|
| **Hit Crystal (HC)** | Places obsidian → detonates crystal with configurable delays |
| **Single Anchor (SA)** | Places Respawn Anchor → charges with Glowstone → detonates |
| **Double Anchor (DA)** | Two rapid anchor detonations in sequence (default 75ms gap) |
| **Anchor Pearl (AP)** | Detonates anchor → throws ender pearl for low-ground advantage |

### Mace & Sword Macros
| Macro | Description |
|-------|-------------|
| **Mace Attack** | Precise-timed mace attack with slot swap |
| **Wind Charge Combo** | Wind charge throw → mace swap → airborne attack |
| **W-Tap** | Release/re-press W during attack for enhanced knockback |
| **Sprint Reset** | Toggle sprint via double-tap W or Ctrl for sprint reset |

### Security & Anti-Detection
- **Streamproof** — `SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE)` hides overlay from OBS/Discord/screen capture
- **PE Header Erasure** — Overwrites PE header in memory to hinder static analysis
- **Module Name Spoofing** — Renames module in PEB to a benign process name
- **Random Window Class** — Randomised WNDCLASS name each launch
- **DWM Window Cloaking** — Hide from task switcher
- **Timer Resolution** — `NtSetTimerResolution` at 0.5ms for precision sleep

### Input Simulation
- **SendInput API** — Low-level keyboard/mouse simulation (harder for anti-cheat to detect vs `PostMessage`/`SendMessage`)
- **Randomised Jitter** — All delays include ±[3-7]ms random variation to mimic human behaviour
- **Precision Sleep** — Spin-wait hybrid for sub-millisecond accuracy

### GUI
- **ImGui + DirectX 11** backend with elegant dark theme
- **6 theme presets**: Dark, Midnight, Carbon, Emerald, Crimson, Ocean
- **Left sidebar** navigation: Crystal, Mace, Sword, Other, Themes, Settings
- **Custom widgets**: Toggle switches, delay sliders, keybind capture buttons, slot selectors
- **Status bar** with connection indicator and version display

---

## Project Structure

```
noqwdmacro/
├── CMakeLists.txt                  # Build system
├── README.md
├── include/
│   ├── config.h                    # Global constants & defaults
│   ├── gui/
│   │   ├── gui.h                   # Main GUI controller
│   │   ├── theme.h                 # Theme engine (6 presets)
│   │   └── widgets.h               # Custom ImGui widgets
│   ├── input/
│   │   └── input_simulator.h       # SendInput wrapper
│   ├── macros/
│   │   ├── macro_base.h            # Abstract macro base class
│   │   ├── crystal.h               # HC, SA, DA, AP macros
│   │   ├── mace.h                  # Mace Attack, Wind Charge
│   │   └── sword.h                 # W-Tap, Sprint Reset
│   ├── security/
│   │   ├── streamproof.h           # WDA_EXCLUDEFROMCAPTURE
│   │   └── stealth.h               # Process hiding, PE erase
│   └── utils/
│       ├── delay.h                 # Humanised delay with jitter
│       ├── keybind.h               # Keybind manager
│       └── timer.h                 # High-resolution timer
├── src/
│   ├── main.cpp                    # WinMain entry point
│   ├── gui/
│   │   ├── gui.cpp                 # GUI implementation
│   │   ├── theme.cpp               # Theme colours
│   │   └── widgets.cpp             # Widget rendering
│   ├── input/
│   │   └── input_simulator.cpp     # SendInput implementation
│   ├── macros/
│   │   ├── crystal.cpp             # Crystal PVP sequences
│   │   ├── mace.cpp                # Mace sequences
│   │   └── sword.cpp               # Sword sequences
│   ├── security/
│   │   ├── streamproof.cpp         # Display affinity
│   │   └── stealth.cpp             # Anti-detection
│   └── utils/
│       ├── delay.cpp               # Jittered sleep
│       ├── keybind.cpp             # Key polling & capture
│       └── timer.cpp               # Chrono timer
└── deps/
    └── imgui/                      # ImGui library (you provide)
```

---

## Dependencies

| Dependency | Version | Purpose | Source |
|------------|---------|---------|--------|
| **ImGui** | Docking branch | GUI rendering | https://github.com/ocornut/imgui |
| **DirectX 11 SDK** | Windows SDK | GPU rendering backend | Included with Windows SDK / Visual Studio |
| **MinHook** *(optional)* | 1.3.3+ | API hooking if needed | https://github.com/TsudaKageworthy/minhook |

### System Requirements
- **OS**: Windows 10 1903+ (for `WDA_EXCLUDEFROMCAPTURE`)
- **Compiler**: MSVC 2019+ or MinGW-w64 with C++17 support
- **GPU**: DirectX 11 capable

---

## Build Instructions

### Prerequisites

1. Install **Visual Studio 2019/2022** with "Desktop development with C++" workload
2. Install **CMake 3.16+** (or use the one bundled with VS)
3. Clone **ImGui** (docking branch) into `deps/imgui/`:

```bash
git clone --branch docking https://github.com/ocornut/imgui.git deps/imgui
```

### Build with CMake

```bash
# From project root
mkdir build && cd build

# Configure (DirectX 11 backend — default)
cmake .. -G "Visual Studio 17 2022" -A x64

# Build Release
cmake --build . --config Release

# Output binary: build/bin/Release/noqwdmacro.exe
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `USE_DIRECTX11` | `ON` | Use DirectX 11 backend |
| `USE_OPENGL3` | `OFF` | Use OpenGL 3 + GLFW backend (requires GLFW) |

```bash
# OpenGL backend (requires GLFW installed)
cmake .. -DUSE_DIRECTX11=OFF -DUSE_OPENGL3=ON
```

### MinHook (Optional)

If you need hooking support, place MinHook in `deps/minhook/`:
```
deps/minhook/
├── include/MinHook.h
└── lib/MinHook.x64.lib
```
The build system auto-detects it and defines `NOQWD_USE_MINHOOK`.

---

## Usage

1. Launch `noqwdmacro.exe`
2. Use the sidebar to navigate between Crystal, Mace, Sword panels
3. **Enable** each macro with the toggle switch
4. **Set delays** using the sliders (in milliseconds)
5. **Assign keybinds** by clicking the keybind button and pressing your desired key
6. **Configure hotbar slots** to match your in-game inventory layout
7. Enable **Streamproof** in the Other panel if streaming/recording

### Tips
- Adjust jitter values in the Other panel for more/less randomisation
- Use the Themes panel to switch between 6 dark themes
- All settings reset on close (persistence can be added via JSON config)

---

## Credits

- **Application**: noqwdmacro
- **Author**: noqwd
- **Version**: v1.4.0
- **GUI Framework**: [Dear ImGui](https://github.com/ocornut/imgui) by Omar Cornut
- **Rendering**: DirectX 11 (Microsoft)

---

## Disclaimer

This software is provided for educational and research purposes only. Use at your own risk. The author is not responsible for any bans, penalties, or consequences resulting from the use of this software in online games. Always check and comply with the terms of service of any game you play.
