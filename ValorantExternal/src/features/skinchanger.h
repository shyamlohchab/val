#pragma once
#include <cstdint>
#include <string>

namespace SkinChanger
{
    struct Config
    {
        bool enabled = false;
        // Map of weapon slot index -> skin asset path
        // e.g. slot 0 = primary, 1 = sidearm, etc.
    };

    extern Config g_cfg;

    // Applies skin overrides to the local player's inventory
    void Apply();

    // Reverts to original skins
    void Revert();
}
