# CrystalSpKMacro

Minecraft Crystal PvP Macro — an Electron desktop application by noqwd.

## Features

- **Crystal Macros**: Auto-place, hand craft, sword refill, and more
- **Sword Macros**: Sword refill, weapon toggle, auto-shield break
- **Triggerbot**: Smart crosshair-based click automation with multiple modes
- **Anti-Detect**: Human-like timing variance (stealth, jitter, pattern break)
- **Stream Proof**: Hide from screen capture when streaming
- **Focus Lock**: Only activates when Minecraft is focused
- **Auto-Updater**: Keeps the app up to date automatically

## Building

### Prerequisites

- [Node.js](https://nodejs.org/) v20+
- Windows OS (for native modules)

### Install & Run (Development)

```bash
npm install
npm start
```

### Build .exe

```bash
# Build both installer and portable
npm run build

# Build portable only
npm run build:portable

# Build installer only
npm run build:installer
```

Built files will be in the `dist/` folder.

### Automated Builds

This repo includes a GitHub Actions workflow that automatically builds `.exe` files on every push to `main`/`master` and on releases. Download built artifacts from the **Actions** tab.

## Project Structure

```
CrystalSpKMacro/
├── electron/          # Electron main process
│   ├── main.js        # App entry point
│   ├── preload.js     # Preload script
│   ├── tray.js        # System tray
│   ├── settings.js    # Settings management
│   └── ...
├── macros/            # Macro modules
│   ├── crystal/       # Crystal PvP macros
│   ├── sword/         # Sword macros
│   ├── ahk/           # AutoHotKey scripts
│   ├── engine.js      # Macro engine
│   ├── triggerbot.js  # Triggerbot module
│   └── ...
├── renderer/          # Frontend UI
│   └── index.html     # Main UI
├── assets/            # Icons and images
└── package.json
```

## Credits

Created by **noqwd**
