#include "aimbot.h"
#include "../sdk.h"
#include "../offsets.h"
#include "../decryption.h"
#include "../math.h"
#include <Windows.h>
#include <cmath>

namespace Aimbot
{
    Config g_cfg;

    // Reads a bone world position from a USkeletalMeshComponent
    static FVector GetBonePos(uintptr_t mesh, int boneIdx)
    {
        uintptr_t boneArray = *reinterpret_cast<uintptr_t*>(
            mesh + Offsets::MESH_COMPONENT_SPACE_BASES);
        if (!boneArray) return {};
        FTransform transform = *reinterpret_cast<FTransform*>(
            boneArray + boneIdx * sizeof(FTransform));
        return transform.GetBoneLocation();
    }

    // Reads the local player's current view rotation via GetPlayerViewPoint
    static FRotator GetViewRotation()
    {
        // Stub — read from PlayerController offset
        uintptr_t gworld = Decryption::GetGWorld();
        uintptr_t gi     = *reinterpret_cast<uintptr_t*>(gworld + Offsets::UWORLD_GAME_INSTANCE);
        uintptr_t lp     = *reinterpret_cast<uintptr_t*>(
            *reinterpret_cast<uintptr_t*>(gi + Offsets::GAME_INSTANCE_LOCAL_PLAYERS)
            + 0x0); // first element
        uintptr_t pc     = *reinterpret_cast<uintptr_t*>(lp + Offsets::LOCAL_PLAYER_CONTROLLER);
        return *reinterpret_cast<FRotator*>(pc + 0x440); // ControlRotation offset
    }

    // Sends a relative mouse move to aim toward target
    static void SetAim(const FRotator& target)
    {
        FRotator current = GetViewRotation();
        FRotator smoothed = Math::SmoothAim(current, target, g_cfg.smooth);

        // Convert delta to mouse units (sensitivity-dependent — tune as needed)
        float sensitivity = 0.022f;
        float dx = Math::ClampAngle(smoothed.Yaw   - current.Yaw)   / sensitivity;
        float dy = Math::ClampAngle(smoothed.Pitch - current.Pitch) / sensitivity;

        mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(dx),
                    static_cast<DWORD>(dy), 0, 0);
    }

    void Tick()
    {
        if (!g_cfg.enabled) return;
        if (!GetAsyncKeyState(VK_RBUTTON)) return; // hold RMB to aim

        uintptr_t gworld = Decryption::GetGWorld();
        if (!gworld) return;

        // Iterate player array from GameState
        uintptr_t gameState = *reinterpret_cast<uintptr_t*>(gworld + 0x170);
        auto& playerArray   = *reinterpret_cast<TArray<uintptr_t>*>(
            gameState + Offsets::GAMESTATE_PLAYER_ARRAY);

        FVector localPos = {}; // TODO: read local pawn root
        float   bestFov  = g_cfg.fov;
        FVector bestBone = {};
        bool    found    = false;

        for (int i = 0; i < playerArray.Count; ++i)
        {
            uintptr_t playerState = playerArray.Read(i);
            if (!playerState) continue;

            uintptr_t pawn = *reinterpret_cast<uintptr_t*>(playerState + 0x308);
            if (!pawn) continue;

            int team = *reinterpret_cast<int*>(pawn + Offsets::PAWN_TEAM_ID);
            // Skip teammates (team 0 = local, same team)
            // TODO: compare against local team ID

            uintptr_t mesh = *reinterpret_cast<uintptr_t*>(pawn + Offsets::PAWN_MESH);
            if (!mesh) continue;

            FVector bonePos = GetBonePos(mesh, g_cfg.targetBone);
            FVector delta   = bonePos - localPos;
            FRotator toTarget = Math::VectorToRotator(delta);
            FRotator viewRot  = GetViewRotation();

            float dPitch = fabsf(Math::ClampAngle(toTarget.Pitch - viewRot.Pitch));
            float dYaw   = fabsf(Math::ClampAngle(toTarget.Yaw   - viewRot.Yaw));
            float fovDist = sqrtf(dPitch * dPitch + dYaw * dYaw);

            if (fovDist < bestFov)
            {
                bestFov  = fovDist;
                bestBone = bonePos;
                found    = true;
            }
        }

        if (found)
        {
            FVector delta = bestBone - localPos;
            FRotator target = Math::VectorToRotator(delta);
            SetAim(target);

            // Triggerbot
            if (g_cfg.triggerbot && bestFov < 0.5f)
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        }
    }
}
