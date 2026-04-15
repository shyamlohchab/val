#pragma once
#include <d3d12.h>
#include <cstdint>

// ============================================================
//  Chams — material override for skeleton/mesh visibility
// ============================================================
namespace Chams
{
    struct Config
    {
        bool  enabled         = false;
        bool  visibleOnly     = false;   // only apply to visible enemies
        bool  wireframe       = false;
        float enemyColor[4]   = { 1.f, 0.f, 0.f, 1.f }; // RGBA
        float teamColor[4]    = { 0.f, 1.f, 0.f, 1.f };
        float hiddenColor[4]  = { 1.f, 0.5f, 0.f, 0.7f };
    };

    extern Config g_cfg;

    // Called once after DX12 device is available
    bool Init(ID3D12Device* device);

    // Called per-frame before scene render to override materials
    void Apply(uintptr_t meshComponent, bool isEnemy, bool isVisible);

    // Restores original materials
    void Restore(uintptr_t meshComponent);

    void Shutdown();
}
