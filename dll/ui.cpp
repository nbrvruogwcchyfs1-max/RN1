#include "ui.hpp"
#include "hooks.hpp"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <dxgi.h>
#include <string>
#include <windows.h>
#include <MinHook.h>

static bool g_show = true;
static char g_script_buf[65536] = "";
static std::string g_console = "";
static bool g_imgui_ready = false;
static ID3D11Device* g_device = nullptr;
static ID3D11DeviceContext* g_context = nullptr;
static HWND g_hwnd = nullptr;

typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain*, UINT, UINT);
static Present_t g_original_present = nullptr;

static void init_imgui(IDXGISwapChain* swap) {
    swap->GetDevice(__uuidof(ID3D11Device), (void**)&g_device);
    g_device->GetImmediateContext(&g_context);
    DXGI_SWAP_CHAIN_DESC desc;
    swap->GetDesc(&desc);
    g_hwnd = desc.OutputWindow;

    ImGui::CreateContext();
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_device, g_context);
    g_imgui_ready = true;
}

static HRESULT __stdcall hooked_present(IDXGISwapChain* swap, UINT sync, UINT flags) {
    if (!g_imgui_ready) init_imgui(swap);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (GetAsyncKeyState(VK_INSERT) & 1) g_show = !g_show;

    if (g_show) {
        ImGui::Begin("AlphaExec", &g_show);
        ImGui::InputTextMultiline("##script", g_script_buf, sizeof(g_script_buf), ImVec2(500, 300));
        if (ImGui::Button("Execute")) {
            hooks::queue_script(std::string(g_script_buf));
            g_console += ">>> queued\n";
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear")) g_script_buf[0] = 0;
        ImGui::BeginChild("##console", ImVec2(500, 120), true);
        ImGui::TextUnformatted(g_console.c_str());
        ImGui::EndChild();
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return g_original_present(swap, sync, flags);
}

namespace ui {

void start() {
    DXGI_SWAP_CHAIN_DESC desc = {};
    desc.BufferCount = 1;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = GetForegroundWindow();
    desc.SampleDesc.Count = 1;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* dummy_swap = nullptr;
    ID3D11Device* dummy_device = nullptr;
    ID3D11DeviceContext* dummy_context = nullptr;

    D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &desc, &dummy_swap, &dummy_device, &dummy_context
    );

    if (!dummy_swap) return;

    void** vtable = *(void***)dummy_swap;
    g_original_present = (Present_t)vtable[8];

    dummy_swap->Release();
    dummy_device->Release();
    dummy_context->Release();

    MH_Initialize();
    MH_CreateHook((void*)g_original_present, &hooked_present, (void**)&g_original_present);
    MH_EnableHook((void*)g_original_present);
}

void stop() {
    if (g_imgui_ready) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_imgui_ready = false;
    }
}

}
