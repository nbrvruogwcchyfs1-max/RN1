cmake_minimum_required(VERSION 3.20)
project(AlphaExec LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ═══ Luau (يُحمّل تلقائياً) ═══
set(LUAU_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    luau
    GIT_REPOSITORY https://github.com/luau-lang/luau.git
    GIT_TAG 0.630
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(luau)

# ═══ MinHook (يُحمّل تلقائياً) ═══
FetchContent_Declare(
    minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG v1.3.4
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(minhook)
if(NOT minhook_POPULATED)
    FetchContent_Populate(minhook)
endif()

# ═══ Dear ImGui (يُحمّل تلقائياً) ═══
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.5
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

# ═══ DLL ═══
add_library(alphaexec SHARED
    dll/dllmain.cpp
    dll/state.cpp
    dll/memory.cpp
    dll/execution.cpp
    dll/hooks.cpp
    dll/ui.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    ${minhook_SOURCE_DIR}/src/hook.c
    ${minhook_SOURCE_DIR}/src/buffer.c
    ${minhook_SOURCE_DIR}/src/trampoline.c
)

target_include_directories(alphaexec PRIVATE
    ${CMAKE_SOURCE_DIR}/dll
    ${minhook_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(alphaexec PRIVATE
    Luau.VM
    Luau.Compiler
    d3d11
    dxgi
    user32
    psapi
)

# ═══ Injector ═══
add_executable(injector injector/main.cpp)
target_link_libraries(injector PRIVATE user32 psapi)
