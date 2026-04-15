#pragma once
#include <d3d12.h>

// ============================================================
//  ImGui in-game menu declarations
// ============================================================
namespace Menu
{
    extern bool g_open;

    // Call after ImGui DX12 backend is initialized
    void Init();

    // Call inside the hooked Present/PostRender
    void Render();

    void Shutdown();
}
