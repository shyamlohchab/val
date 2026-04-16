#include "skinchanger.h"
#include "../sdk.h"
#include "../offsets.h"
#include "../decryption.h"
#include "../game/UAresInventory.h"

// ============================================================
//  SkinChanger
//  Reads the local player's inventory component and patches
//  the skin asset soft-object reference for each equipped weapon.
//  Client-side only — other players see original skins.
// ============================================================

namespace SkinChanger
{
    Config g_cfg;

    // Offset to UAresInventoryComponent* on the local pawn
    // TODO: update from IDA per patch
    static constexpr uint32_t INVENTORY_COMP_OFFSET = 0x8A8;

    static uintptr_t GetLocalPawn()
    {
        uintptr_t gworld = Decryption::GetGWorld();
        if (!gworld) return 0;
        uintptr_t gi = *reinterpret_cast<uintptr_t*>(gworld + Offsets::UWORLD_GAME_INSTANCE);
        if (!gi) return 0;
        uintptr_t localPlayersPtr = *reinterpret_cast<uintptr_t*>(gi + Offsets::GAME_INSTANCE_LOCAL_PLAYERS);
        if (!localPlayersPtr) return 0;
        uintptr_t lp = *reinterpret_cast<uintptr_t*>(localPlayersPtr);
        if (!lp) return 0;
        uintptr_t pc = *reinterpret_cast<uintptr_t*>(lp + Offsets::LOCAL_PLAYER_CONTROLLER);
        if (!pc) return 0;
        return *reinterpret_cast<uintptr_t*>(pc + Offsets::PLAYER_CONTROLLER_PAWN);
    }

    void Apply()
    {
        if (!g_cfg.enabled) return;

        uintptr_t pawn = GetLocalPawn();
        if (!pawn) return;

        uintptr_t inventoryComp = *reinterpret_cast<uintptr_t*>(pawn + INVENTORY_COMP_OFFSET);
        if (!inventoryComp) return;

        // Iterate weapon slots and override skin paths
        // g_cfg.skinMap maps EWeaponSlot -> desired skin asset path (wstring)
        for (auto& [slot, path] : g_cfg.skinMap)
        {
            uintptr_t equippable = Inventory::GetEquippable(
                inventoryComp, static_cast<Inventory::EWeaponSlot>(slot));
            if (!equippable) continue;
            Inventory::SetSkinPath(equippable, path);
        }
    }

    void Revert()
    {
        if (!g_cfg.enabled) return;

        uintptr_t pawn = GetLocalPawn();
        if (!pawn) return;

        uintptr_t inventoryComp = *reinterpret_cast<uintptr_t*>(pawn + INVENTORY_COMP_OFFSET);
        if (!inventoryComp) return;

        for (auto& [slot, path] : g_cfg.originalSkinMap)
        {
            uintptr_t equippable = Inventory::GetEquippable(
                inventoryComp, static_cast<Inventory::EWeaponSlot>(slot));
            if (!equippable) continue;
            Inventory::SetSkinPath(equippable, path);
        }
    }
}
