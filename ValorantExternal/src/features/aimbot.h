#pragma once
#include "../sdk.h"

namespace Aimbot
{
    struct Config
    {
        bool  enabled      = false;
        bool  visibleOnly  = true;
        int   targetBone   = 8;    // 8 = head, 5 = chest
        float fov          = 5.f;  // degrees
        float smooth       = 8.f;  // higher = smoother
        bool  triggerbot   = false;
        float triggerDelay = 0.05f; // seconds
    };

    extern Config g_cfg;

    // Called every frame from hkPostRender
    void Tick();

    // Returns the closest enemy bone position within FOV, or nullptr
    FVector* GetBestTarget();
}
