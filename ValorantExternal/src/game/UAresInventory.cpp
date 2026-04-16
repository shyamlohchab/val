#include "UAresInventory.h"
#include "../offsets.h"
#include "../decryption.h"

// ============================================================
//  UAresInventoryComponent + UEquippableComponent
// ============================================================

namespace Inventory
{
    uintptr_t GetEquippable(uintptr_t inventoryComp, EWeaponSlot slot)
    {
        // TArray<UEquippableComponent*> at some offset — patch specific
        constexpr uint32_t EQUIPPABLES_OFFSET = 0x4B8; // TODO: update per patch
        auto& arr = *reinterpret_cast<TArray<uintptr_t>*>(
            inventoryComp + EQUIPPABLES_OFFSET);
        if (slot >= arr.Count) return 0;
        return arr.Read(static_cast<int>(slot));
    }

    std::string GetSkinPath(uintptr_t equippable)
    {
        constexpr uint32_t SKIN_PATH_OFFSET = 0x1C0; // TODO: update per patch
        auto* str = reinterpret_cast<FString*>(equippable + SKIN_PATH_OFFSET);
        return str->ToString();
    }

    void SetSkinPath(uintptr_t equippable, const std::wstring& path)
    {
        constexpr uint32_t SKIN_PATH_OFFSET = 0x1C0;
        auto* str = reinterpret_cast<FString*>(equippable + SKIN_PATH_OFFSET);
        // Only safe if new path fits in existing allocation.
        // Caller must ensure capacity or allocate via FMemory::Malloc.
        wcsncpy_s(reinterpret_cast<wchar_t*>(str->Data),
                  str->Max, path.c_str(), path.size());
        str->Count = static_cast<int32_t>(path.size() + 1);
    }
}
