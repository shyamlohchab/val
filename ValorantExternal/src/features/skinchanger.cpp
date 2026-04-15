#include "skinchanger.h"
#include "../sdk.h"
#include "../offsets.h"
#include "../decryption.h"

namespace SkinChanger
{
    Config g_cfg;

    // UAresInventoryComponent + UEquippableComponent skin override
    // Reads the local player's inventory component and patches
    // the skin asset soft-object reference for each equipped weapon.

    void Apply()
    {
        if (!g_cfg.enabled) return;

        uintptr_t gworld = Decryption::GetGWorld();
        if (!gworld) return;

        // TODO: walk local player -> pawn -> inventory component
        // -> weapon slots -> override skin asset path
        // Offsets are patch-specific; update from IDA/Ghidra.
    }

    void Revert()
    {
        // TODO: restore original skin asset paths
    }
}
