cmake_minimum_required(VERSION 3.20)
project(CrystalSpKMacro VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimize for speed
if(MSVC)
    add_compile_options(/O2 /Oi /GL /GS- /fp:fast /EHsc /W3)
    add_link_options(/LTCG /OPT:REF /OPT:ICF)
else()
    add_compile_options(-O3 -march=native -flto)
endif()

# ── Sources ──────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/input.cpp
    src/macros.cpp
    src/engine.cpp
    src/config.cpp
    src/gui.cpp
    src/tray.cpp
    src/optimizer.cpp
)

# ── Executable ───────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} resources/app.rc)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    vendor
)

# ── Win32 libraries ──────────────────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME} PRIVATE
    user32
    kernel32
    gdi32
    shell32
    advapi32
    comctl32
    ole32
    winmm
    ws2_32
    dwmapi
    uxtheme
    shlwapi
)

# ── Definitions ──────────────────────────────────────────────────────────────
target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    APP_VERSION="${PROJECT_VERSION}"
)

# ── Static runtime (single .exe, no VCRUNTIME DLL needed) ───────────────────
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
