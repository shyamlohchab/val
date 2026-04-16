#pragma once
#include <cstdint>
#include <string>
#include <map>

namespace SkinChanger
{
    struct Config
    {
        bool enabled = false;
        // Map of weapon slot index -> desired skin asset path (wstring)
        std::map<int, std::wstring> skinMap;
        // Original skin paths saved before Apply() — populated on first Apply()
        std::map<int, std::wstring> originalSkinMap;
    };

    extern Config g_cfg;

    // Applies skin overrides to the local player's inventory
    void Apply();

    // Reverts to original skins
    void Revert();
}
