#include "dx12hook.h"
#include "menu.h"
#include "features/esp.h"
#include "features/aimbot.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>
#include <Windows.h>

// ============================================================
//  DX12Hook — VMT hook on IDXGISwapChain3::Present
//  Strategy: create a dummy D3D12 device + swap chain,
//  read the VMT, patch Present slot (index 8) and
//  ResizeBuffers slot (index 13).
// ============================================================

using PresentFn       = HRESULT(WINAPI*)(IDXGISwapChain3*, UINT, UINT);
using ResizeBuffersFn = HRESULT(WINAPI*)(IDXGISwapChain3*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

static PresentFn       g_oPresent       = nullptr;
static ResizeBuffersFn g_oResizeBuffers = nullptr;

static IDXGISwapChain3*         g_swapChain    = nullptr;
static ID3D12Device*            g_device       = nullptr;
static ID3D12DescriptorHeap*    g_srvHeap      = nullptr;
static ID3D12DescriptorHeap*    g_rtvHeap      = nullptr;
static ID3D12CommandAllocator*  g_cmdAlloc     = nullptr;
static ID3D12GraphicsCommandList* g_cmdList    = nullptr;
static ID3D12CommandQueue*      g_cmdQueue     = nullptr;
static bool                     g_imguiReady   = false;
static UINT                     g_bufferCount  = 0;

// Per-frame render targets
struct FrameContext
{
    ID3D12CommandAllocator* CmdAlloc;
    ID3D12Resource*         RenderTarget;
    D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle;
};
static FrameContext* g_frames = nullptr;

// ---- hooked Present ----------------------------------------
static HRESULT WINAPI HookedPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    DX12Hook::OnPresent(pSwapChain, SyncInterval, Flags);
    return g_oPresent(pSwapChain, SyncInterval, Flags);
}

// ---- hooked ResizeBuffers ----------------------------------
static HRESULT WINAPI HookedResizeBuffers(
    IDXGISwapChain3* pSwapChain, UINT BufferCount,
    UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    // Invalidate per-frame resources before resize
    if (g_imguiReady)
    {
        ImGui_ImplDX12_InvalidateDeviceObjects();
        if (g_frames)
        {
            for (UINT i = 0; i < g_bufferCount; ++i)
            {
                if (g_frames[i].RenderTarget)
                    g_frames[i].RenderTarget->Release();
            }
            delete[] g_frames;
            g_frames = nullptr;
        }
    }
    return g_oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

// ---- patch a VMT slot --------------------------------------
static void PatchVMT(void** vmt, int slot, void* newFn, void** outOrig)
{
    DWORD old;
    VirtualProtect(&vmt[slot], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    *outOrig = vmt[slot];
    vmt[slot] = newFn;
    VirtualProtect(&vmt[slot], sizeof(void*), old, &old);
}

bool DX12Hook::Init()
{
    // Create a temporary D3D12 device + DXGI swap chain to steal the VMT
    WNDCLASSEXA wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = DefWindowProcA;
    wc.lpszClassName = "DX12Dummy";
    RegisterClassExA(&wc);
    HWND hWnd = CreateWindowExA(0, "DX12Dummy", nullptr, WS_OVERLAPPED,
                                0, 0, 1, 1, nullptr, nullptr, nullptr, nullptr);
    if (!hWnd) return false;

    ID3D12Device* tmpDev = nullptr;
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
                                 IID_PPV_ARGS(&tmpDev))))
    {
        DestroyWindow(hWnd);
        UnregisterClassA("DX12Dummy", nullptr);
        return false;
    }

    ID3D12CommandQueue* tmpQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC cqd{};
    cqd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    tmpDev->CreateCommandQueue(&cqd, IID_PPV_ARGS(&tmpQueue));

    IDXGIFactory4* factory = nullptr;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount        = 2;
    scd.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow       = hWnd;
    scd.SampleDesc.Count   = 1;
    scd.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.Windowed           = TRUE;

    IDXGISwapChain* tmpChain = nullptr;
    factory->CreateSwapChain(tmpQueue, &scd, &tmpChain);

    // VMT slots: Present=8, ResizeBuffers=13
    void** vmt = *reinterpret_cast<void***>(tmpChain);
    PatchVMT(vmt, 8,  reinterpret_cast<void*>(HookedPresent),
             reinterpret_cast<void**>(&g_oPresent));
    PatchVMT(vmt, 13, reinterpret_cast<void*>(HookedResizeBuffers),
             reinterpret_cast<void**>(&g_oResizeBuffers));

    // Cleanup dummy objects
    tmpChain->Release();
    factory->Release();
    tmpQueue->Release();
    tmpDev->Release();
    DestroyWindow(hWnd);
    UnregisterClassA("DX12Dummy", nullptr);
    return true;
}

void DX12Hook::OnPresent(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags)
{
    (void)syncInterval; (void)flags;

    if (FAILED(swapChain->GetDevice(IID_PPV_ARGS(&g_device))))
        return;

    if (!g_imguiReady)
    {
        // Query buffer count
        DXGI_SWAP_CHAIN_DESC desc{};
        swapChain->GetDesc(&desc);
        g_bufferCount = desc.BufferCount;

        // SRV heap for ImGui
        D3D12_DESCRIPTOR_HEAP_DESC srvDesc{};
        srvDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvDesc.NumDescriptors = 1;
        srvDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        g_device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&g_srvHeap));

        // RTV heap
        D3D12_DESCRIPTOR_HEAP_DESC rtvDesc{};
        rtvDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvDesc.NumDescriptors = g_bufferCount;
        g_device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&g_rtvHeap));

        // Per-frame contexts
        g_frames = new FrameContext[g_bufferCount]{};
        UINT rtvSize = g_device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
            g_rtvHeap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0; i < g_bufferCount; ++i)
        {
            g_device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&g_frames[i].CmdAlloc));
            swapChain->GetBuffer(i, IID_PPV_ARGS(&g_frames[i].RenderTarget));
            g_device->CreateRenderTargetView(g_frames[i].RenderTarget,
                                             nullptr, rtvHandle);
            g_frames[i].RtvHandle = rtvHandle;
            rtvHandle.ptr += rtvSize;
        }

        // Command list
        g_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_cmdAlloc));
        g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                    g_cmdAlloc, nullptr, IID_PPV_ARGS(&g_cmdList));
        g_cmdList->Close();

        // Command queue — grab from swap chain
        // (We stored it during Init; simplest approach: re-query via DXGI)
        IDXGIDevice* dxgiDev = nullptr;
        g_device->QueryInterface(IID_PPV_ARGS(&dxgiDev));
        if (dxgiDev) dxgiDev->Release();

        // Init ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplWin32_Init(desc.OutputWindow);
        ImGui_ImplDX12_Init(g_device, static_cast<int>(g_bufferCount),
            DXGI_FORMAT_R8G8B8A8_UNORM,
            g_srvHeap,
            g_srvHeap->GetCPUDescriptorHandleForHeapStart(),
            g_srvHeap->GetGPUDescriptorHandleForHeapStart());

        Menu::Init();
        g_imguiReady = true;
    }

    // Run feature logic
    ESP::Tick();
    Aimbot::Tick();

    // Render ImGui overlay
    UINT backBufIdx = swapChain->GetCurrentBackBufferIndex();
    FrameContext& frame = g_frames[backBufIdx];

    frame.CmdAlloc->Reset();
    g_cmdList->Reset(frame.CmdAlloc, nullptr);

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = frame.RenderTarget;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    g_cmdList->ResourceBarrier(1, &barrier);

    g_cmdList->OMSetRenderTargets(1, &frame.RtvHandle, FALSE, nullptr);
    g_cmdList->SetDescriptorHeaps(1, &g_srvHeap);

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ESP::Render();
    Menu::Render();

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_cmdList);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    g_cmdList->ResourceBarrier(1, &barrier);
    g_cmdList->Close();

    // NOTE: Submit via the real command queue.
    // The queue pointer must be captured during Init or passed in.
    // Left as exercise — hook IDXGISwapChain::GetContainingOutput or
    // store the queue from the dummy device creation path.

    g_device->Release();
}

void DX12Hook::Shutdown()
{
    if (g_imguiReady)
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_imguiReady = false;
    }
    if (g_frames)
    {
        for (UINT i = 0; i < g_bufferCount; ++i)
        {
            if (g_frames[i].CmdAlloc)    g_frames[i].CmdAlloc->Release();
            if (g_frames[i].RenderTarget) g_frames[i].RenderTarget->Release();
        }
        delete[] g_frames;
        g_frames = nullptr;
    }
    if (g_cmdList)  { g_cmdList->Release();  g_cmdList  = nullptr; }
    if (g_cmdAlloc) { g_cmdAlloc->Release(); g_cmdAlloc = nullptr; }
    if (g_rtvHeap)  { g_rtvHeap->Release();  g_rtvHeap  = nullptr; }
    if (g_srvHeap)  { g_srvHeap->Release();  g_srvHeap  = nullptr; }

    // Restore VMT slots
    if (g_oPresent)
    {
        // Swap chain is gone by now; nothing to restore safely.
        // Hooks are removed when the process exits anyway.
        g_oPresent       = nullptr;
        g_oResizeBuffers = nullptr;
    }
}
