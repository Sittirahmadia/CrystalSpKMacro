cmake_minimum_required(VERSION 3.16)
project(noqwdmacro VERSION 1.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ─── Options ──────────────────────────────────────────────
option(USE_DIRECTX11 "Use DirectX 11 backend (Windows only)" ON)
option(USE_OPENGL3   "Use OpenGL 3 + GLFW backend"           OFF)

# ─── ImGui Sources ────────────────────────────────────────
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/deps/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

if(USE_DIRECTX11)
    list(APPEND IMGUI_SOURCES
        ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
        ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    )
elseif(USE_OPENGL3)
    list(APPEND IMGUI_SOURCES
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
endif()

# ─── Application Sources ─────────────────────────────────
set(APP_SOURCES
    src/main.cpp
    src/gui/gui.cpp
    src/gui/theme.cpp
    src/gui/widgets.cpp
    src/macros/crystal.cpp
    src/macros/mace.cpp
    src/macros/sword.cpp
    src/input/input_simulator.cpp
    src/security/streamproof.cpp
    src/security/stealth.cpp
    src/utils/delay.cpp
    src/utils/keybind.cpp
    src/utils/timer.cpp
)

# ─── Executable ───────────────────────────────────────────
add_executable(${PROJECT_NAME} WIN32 ${APP_SOURCES} ${IMGUI_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ─── Platform Libraries ──────────────────────────────────
if(USE_DIRECTX11)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOQWD_USE_DX11)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        d3d11 d3dcompiler dxgi dwmapi
    )
elseif(USE_OPENGL3)
    find_package(glfw3 3.3 REQUIRED)
    find_package(OpenGL REQUIRED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOQWD_USE_OPENGL3)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw OpenGL::GL
    )
endif()

# ─── MinHook (optional) ──────────────────────────────────
if(EXISTS ${CMAKE_SOURCE_DIR}/deps/minhook/include/MinHook.h)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/deps/minhook/include)
    target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/deps/minhook/lib)
    target_link_libraries(${PROJECT_NAME} PRIVATE MinHook.x64)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOQWD_USE_MINHOOK)
endif()

# ─── Compiler flags ──────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /O2 /GL)
    target_link_options(${PROJECT_NAME} PRIVATE /LTCG /OPT:REF /OPT:ICF)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O2)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    NOQWD_VERSION="${PROJECT_VERSION}"
    UNICODE _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)
