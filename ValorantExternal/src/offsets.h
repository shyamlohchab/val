#pragma once
#include <cstdint>

// ============================================================
//  Offsets — update per Valorant patch
//  Last verified: patch placeholder
// ============================================================
namespace Offsets
{
    // Encrypted static pointer offsets (relative to ShooterGame base)
    constexpr uintptr_t GWORLD_ENCRYPTED   = 0x0;  // TODO: update per patch
    constexpr uintptr_t GOBJECTS_ENCRYPTED = 0x0;  // TODO: update per patch
    constexpr uintptr_t GNAMES_ENCRYPTED   = 0x0;  // TODO: update per patch

    // Decryption key stub (XOR constant, patch-specific)
    constexpr uintptr_t DECRYPT_KEY        = 0x0;  // TODO: update per patch

    // UWorld
    constexpr uint32_t UWORLD_GAME_INSTANCE        = 0x1A0;
    constexpr uint32_t GAME_INSTANCE_LOCAL_PLAYERS = 0x38;
    constexpr uint32_t LOCAL_PLAYER_CONTROLLER     = 0x30;
    constexpr uint32_t PLAYER_CONTROLLER_PAWN      = 0x2A0;

    // APawn / ACharacter
    constexpr uint32_t PAWN_MESH                   = 0x280;
    constexpr uint32_t PAWN_HEALTH                 = 0x1B8;
    constexpr uint32_t PAWN_TEAM_ID                = 0x3C8;
    constexpr uint32_t PAWN_PLAYER_STATE           = 0x2B8;
    constexpr uint32_t PLAYER_STATE_NAME           = 0x3A8;

    // USkeletalMeshComponent
    constexpr uint32_t MESH_COMPONENT_SPACE_BASES  = 0x5B0;
    constexpr uint32_t MESH_BONE_COUNT             = 0x5C0;

    // AGameStateBase
    constexpr uint32_t GAMESTATE_PLAYER_ARRAY      = 0x2A0;

    // Bone indices
    constexpr int BONE_HEAD   = 8;
    constexpr int BONE_NECK   = 7;
    constexpr int BONE_CHEST  = 5;
    constexpr int BONE_PELVIS = 0;
    constexpr int BONE_LHAND  = 32;
    constexpr int BONE_RHAND  = 62;
    constexpr int BONE_LFOOT  = 78;
    constexpr int BONE_RFOOT  = 98;
}
