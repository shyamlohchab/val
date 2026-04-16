#include "../chams.h"
#include "../offsets.h"
#include <d3d12.h>

// ============================================================
//  Chams — material override for skeleton/mesh visibility
//
//  Real implementation requires either:
//    (a) Patching the USkeletalMeshComponent's material array
//        via UE4 internals (UMaterialInstanceDynamic::SetVectorParam)
//    (b) A custom DX12 PSO override injected into the render pipeline.
//
//  This file provides the correct interface + config storage.
//  PSO override path is left as a patch-specific extension.
// ============================================================

namespace Chams
{
    Config g_cfg;

    // Cached DX12 device for future PSO creation
    static ID3D12Device* g_device = nullptr;

    bool Init(ID3D12Device* device)
    {
        g_device = device;
        // TODO: create custom flat-color PSO for chams material
        // Steps:
        //   1. Compile a simple flat-color VS/PS pair
        //   2. Create ID3D12PipelineState with depth test disabled (for wallhack)
        //      and depth test enabled (for visible-only)
        //   3. Store both PSOs for Apply() to swap
        return true;
    }

    void Apply(uintptr_t meshComponent, bool isEnemy, bool isVisible)
    {
        if (!g_cfg.enabled) return;
        if (g_cfg.visibleOnly && !isVisible) return;
        (void)meshComponent; (void)isEnemy;
        // TODO: bind chams PSO and override material slots on
        //       USkeletalMeshComponent via UE4 render proxy.
        //       Offset to material array is patch-specific.
    }

    void Restore(uintptr_t meshComponent)
    {
        (void)meshComponent;
        // TODO: restore original materials / unbind chams PSO
    }

    void Shutdown()
    {
        // Release DX12 PSO resources if created
        g_device = nullptr;
    }
}
