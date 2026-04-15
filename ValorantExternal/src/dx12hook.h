#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

// ============================================================
//  DX12 Present hook — VMT-based
// ============================================================
namespace DX12Hook
{
    // Finds the swap chain, hooks IDXGISwapChain::Present and
    // IDXGISwapChain::ResizeBuffers via VMT patching.
    bool Init();

    // Removes hooks and releases resources
    void Shutdown();

    // Exposed so hkPostRender can call ImGui + feature renders
    void OnPresent(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags);
}
