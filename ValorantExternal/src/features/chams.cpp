#include "../chams.h"
#include "../offsets.h"
#include <d3d12.h>

namespace Chams
{
    Config g_cfg;

    // Placeholder — real implementation requires patching the
    // USkeletalMeshComponent's material array via UE4 internals
    // or a custom DX12 PSO override.

    bool Init(ID3D12Device* device)
    {
        (void)device;
        // TODO: create custom PSO for chams material
        return true;
    }

    void Apply(uintptr_t meshComponent, bool isEnemy, bool isVisible)
    {
        if (!g_cfg.enabled) return;
        (void)meshComponent; (void)isEnemy; (void)isVisible;
        // TODO: override material slots on USkeletalMeshComponent
    }

    void Restore(uintptr_t meshComponent)
    {
        (void)meshComponent;
        // TODO: restore original materials
    }

    void Shutdown()
    {
        // Release any DX12 resources
    }
}
