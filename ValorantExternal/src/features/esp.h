#pragma once
#include "../sdk.h"

namespace ESP
{
    struct Config
    {
        bool  enabled       = false;
        bool  boxes         = true;
        bool  skeleton      = true;
        bool  names         = true;
        bool  health        = true;
        bool  distance      = true;
        bool  snaplines     = false;
        bool  visibleCheck  = true;
        float enemyColor[4] = { 1.f, 0.f, 0.f, 1.f };
        float teamColor[4]  = { 0.f, 1.f, 0.f, 1.f };
        float maxDistance   = 500.f; // meters
    };

    extern Config g_cfg;

    // Updates cached player data — call before Render()
    void Tick();

    // Draws all ESP elements via ImGui draw list
    void Render();
}
