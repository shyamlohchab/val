#pragma once
#include <cstdint>

// =============================================================================
//  Offsets — updated latest patch
//  Base: ShooterGame.exe
// =============================================================================
namespace Offsets
{
    // -------------------------------------------------------------------------
    //  Static pointers (relative to ShooterGame base)
    // -------------------------------------------------------------------------
    constexpr uintptr_t GWORLD           = 0xC1AD2A0;   // UWorld*
    constexpr uintptr_t FNAME_POOL       = 0xC34B800;   // FNamePool

    // -------------------------------------------------------------------------
    //  Encrypted pointer slots (read these raw, then DecryptPointer())
    // -------------------------------------------------------------------------
    constexpr uintptr_t GWORLD_ENCRYPTED    = 0xC1AD2A0;   // same slot, encrypted at runtime
    constexpr uintptr_t GOBJECTS_ENCRYPTED  = 0xC1C4500;   // GObjects encrypted ptr
    constexpr uintptr_t GNAMES_ENCRYPTED    = 0xC34B800;   // GNames  encrypted ptr

    // -------------------------------------------------------------------------
    //  Decryption key (XOR constant — update from IDA per patch)
    // -------------------------------------------------------------------------
    constexpr uintptr_t DECRYPT_KEY         = 0x0;          // patch-specific, update from IDA

    // -------------------------------------------------------------------------
    //  Engine function pointers (relative to ShooterGame base)
    // -------------------------------------------------------------------------
    constexpr uintptr_t ProcessEvent         = 0x1B74230;
    constexpr uintptr_t StaticFindObject     = 0x1BA05C0;
    constexpr uintptr_t StaticLoadObject     = 0x1BA40F0;
    constexpr uintptr_t FMemoryMalloc        = 0x1747C10;
    constexpr uintptr_t ToVectorAndNormalize = 0x1847290;
    constexpr uintptr_t ToAngleAndNormalize  = 0x1841A90;

    // -------------------------------------------------------------------------
    //  Rendering / bone
    // -------------------------------------------------------------------------
    constexpr uintptr_t BoneMatrix          = 0x40EBC10;
    constexpr uintptr_t SetOutlineMode      = 0x407F040;

    // -------------------------------------------------------------------------
    //  Weapon / firing
    // -------------------------------------------------------------------------
    constexpr uintptr_t TriggerVeh                    = 0x177F496;
    constexpr uintptr_t PlayFinisher                  = 0x62424D0;
    constexpr uintptr_t GetSpreadValues               = 0x62DEAA0;
    constexpr uintptr_t GetSpreadAngles               = 0x6F04280;
    constexpr uintptr_t GetFiringLocationAndDirection = 0x6AA7A30;

    // -------------------------------------------------------------------------
    //  UWorld chain
    // -------------------------------------------------------------------------
    constexpr uint32_t UWORLD_GAME_INSTANCE        = 0x1A0;
    constexpr uint32_t GAME_INSTANCE_LOCAL_PLAYERS = 0x38;
    constexpr uint32_t LOCAL_PLAYER_CONTROLLER     = 0x30;
    constexpr uint32_t PLAYER_CONTROLLER_PAWN      = 0x2A0;

    // -------------------------------------------------------------------------
    //  UWorld -> GameState
    // -------------------------------------------------------------------------
    constexpr uint32_t UWORLD_GAME_STATE           = 0x170;  // AGameStateBase*

    // -------------------------------------------------------------------------
    //  APawn / ACharacter
    // -------------------------------------------------------------------------
    constexpr uint32_t PAWN_MESH         = 0x280;
    constexpr uint32_t PAWN_HEALTH       = 0x1B8;
    constexpr uint32_t PAWN_TEAM_ID      = 0x3C8;
    constexpr uint32_t PAWN_PLAYER_STATE = 0x2B8;
    constexpr uint32_t PLAYER_STATE_NAME = 0x3A8;

    // -------------------------------------------------------------------------
    //  USkeletalMeshComponent
    // -------------------------------------------------------------------------
    constexpr uint32_t MESH_COMPONENT_SPACE_BASES = 0x5B0;
    constexpr uint32_t MESH_BONE_COUNT            = 0x5C0;

    // -------------------------------------------------------------------------
    //  AGameStateBase
    // -------------------------------------------------------------------------
    constexpr uint32_t GAMESTATE_PLAYER_ARRAY      = 0x2A0;

    // -------------------------------------------------------------------------
    //  Bone indices
    // -------------------------------------------------------------------------
    constexpr int BONE_HEAD   = 8;
    constexpr int BONE_NECK   = 7;
    constexpr int BONE_CHEST  = 5;
    constexpr int BONE_PELVIS = 0;
    constexpr int BONE_LHAND  = 32;
    constexpr int BONE_RHAND  = 62;
    constexpr int BONE_LFOOT  = 78;
    constexpr int BONE_RFOOT  = 98;
}
