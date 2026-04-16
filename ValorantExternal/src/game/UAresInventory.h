#pragma once
#include "../sdk.h"
#include <string>

// ============================================================
//  UAresInventoryComponent + UEquippableComponent
//  Handles reading/writing weapon/skin data from the
//  local player's inventory.
// ============================================================

namespace Inventory
{
    enum EWeaponSlot : int
    {
        Primary   = 0,
        Secondary = 1,
        Melee     = 2,
        Ability1  = 3,
        Ability2  = 4,
        Ultimate  = 5,
    };

    // Returns the UEquippableComponent* for a given slot
    uintptr_t GetEquippable(uintptr_t inventoryComp, EWeaponSlot slot);

    // Reads the skin asset path string from an equippable
    std::string GetSkinPath(uintptr_t equippable);

    // Overwrites the skin asset path (client-side only)
    void SetSkinPath(uintptr_t equippable, const std::wstring& path);
}
