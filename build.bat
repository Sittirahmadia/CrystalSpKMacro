@echo off
REM ─────────────────────────────────────────────────────────────────────────────
REM  build.bat — Build CrystalSpKMacro native .exe
REM
REM  Prerequisites:
REM    - Visual Studio 2022 (or Build Tools) with C++ workload
REM    - CMake 3.20+ (included with VS)
REM
REM  Usage:
REM    build.bat          — Release build
REM    build.bat debug    — Debug build
REM    build.bat clean    — Clean build directory
REM ─────────────────────────────────────────────────────────────────────────────

setlocal

set BUILD_DIR=build
set BUILD_TYPE=Release

if "%1"=="debug" set BUILD_TYPE=Debug
if "%1"=="clean" (
    echo Cleaning build directory...
    rmdir /s /q %BUILD_DIR% 2>nul
    echo Done.
    exit /b 0
)

echo.
echo ══════════════════════════════════════════════════════════════
echo   CrystalSpKMacro Native Build (%BUILD_TYPE%)
echo ══════════════════════════════════════════════════════════════
echo.

REM Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

REM Configure with CMake
echo [1/2] Configuring with CMake...
cmake -S . -B %BUILD_DIR% -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed.
    echo Make sure Visual Studio 2022 with C++ workload is installed.
    exit /b 1
)

REM Build
echo.
echo [2/2] Building...
cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel
if errorlevel 1 (
    echo.
    echo ERROR: Build failed.
    exit /b 1
)

echo.
echo ══════════════════════════════════════════════════════════════
echo   Build successful!
echo   Output: %BUILD_DIR%\%BUILD_TYPE%\CrystalSpKMacro.exe
echo ══════════════════════════════════════════════════════════════
echo.

REM Show file size
for %%A in (%BUILD_DIR%\%BUILD_TYPE%\CrystalSpKMacro.exe) do (
    set SIZE=%%~zA
    echo   Size: %%~zA bytes
)
echo.
