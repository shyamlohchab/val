#include "dx12hook.h"
#include "menu.h"
#include "features/esp.h"
#include "features/aimbot.h"
#include "features/chams.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>

// ============================================================
//  hkPostRender — called from the hooked DX12 Present
// ============================================================

static bool g_imguiInitialized = false;

static ID3D12DescriptorHeap* g_srvHeap    = nullptr;
static ID3D12DescriptorHeap* g_rtvHeap    = nullptr;
static ID3D12CommandAllocator* g_cmdAlloc = nullptr;
static ID3D12GraphicsCommandList* g_cmdList = nullptr;

void DX12Hook::OnPresent(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags)
{
    (void)syncInterval; (void)flags;

    ID3D12Device* device = nullptr;
    if (FAILED(swapChain->GetDevice(IID_PPV_ARGS(&device))))
        return;

    if (!g_imguiInitialized)
    {
        // Create SRV heap for ImGui
        D3D12_DESCRIPTOR_HEAP_DESC srvDesc{};
        srvDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvDesc.NumDescriptors = 1;
        srvDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&g_srvHeap));

        // Init ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplWin32_Init(FindWindowA(nullptr, "VALORANT"));
        ImGui_ImplDX12_Init(device, 2,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            g_srvHeap,
            g_srvHeap->GetCPUDescriptorHandleForHeapStart(),
            g_srvHeap->GetGPUDescriptorHandleForHeapStart());

        Menu::Init();
        g_imguiInitialized = true;
    }

    // Run feature logic
    ESP::Tick();
    Aimbot::Tick();

    // Render ImGui overlay
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ESP::Render();   // draw ESP overlays via ImGui draw list
    Menu::Render();  // draw menu window

    ImGui::Render();
    // Note: command list submission is handled by the full DX12Hook::Init impl

    device->Release();
}
