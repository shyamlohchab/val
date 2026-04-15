#include "../sdk.h"
#include "../offsets.h"
#include "../decryption.h"

// ============================================================
//  UAresInventoryComponent + UEquippableComponent
//  Handles reading/writing weapon/skin data from the
//  local player's inventory.
// ============================================================

namespace Inventory
{
    // Slot indices (Valorant-specific, verify per patch)
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
    uintptr_t GetEquippable(uintptr_t inventoryComp, EWeaponSlot slot)
    {
        // TArray<UEquippableComponent*> at some offset — patch specific
        constexpr uint32_t EQUIPPABLES_OFFSET = 0x4B8; // TODO: update per patch
        auto& arr = *reinterpret_cast<TArray<uintptr_t>*>(
            inventoryComp + EQUIPPABLES_OFFSET);
        if (slot >= arr.Count) return 0;
        return arr.Read(static_cast<int>(slot));
    }

    // Reads the skin asset path string from an equippable
    std::string GetSkinPath(uintptr_t equippable)
    {
        constexpr uint32_t SKIN_PATH_OFFSET = 0x1C0; // TODO: update per patch
        auto* str = reinterpret_cast<FString*>(equippable + SKIN_PATH_OFFSET);
        return str->ToString();
    }

    // Overwrites the skin asset path (client-side only)
    void SetSkinPath(uintptr_t equippable, const std::wstring& path)
    {
        constexpr uint32_t SKIN_PATH_OFFSET = 0x1C0;
        auto* str = reinterpret_cast<FString*>(equippable + SKIN_PATH_OFFSET);
        // Overwrite the Data pointer and Count — only safe if new path
        // fits in the existing allocation or you allocate new memory.
        // Simplified stub:
        wcsncpy_s(reinterpret_cast<wchar_t*>(str->Data),
                  str->Max, path.c_str(), path.size());
        str->Count = static_cast<int32_t>(path.size() + 1);
    }
}
