#pragma once
#include "../sdk.h"
#include "../offsets.h"

// ============================================================
//  Valorant / UE4 game class wrappers
//  Offsets are patch-specific — update from IDA/Ghidra.
// ============================================================

struct UWorld;
struct AGameStateBase;
struct APlayerController;
struct APawn;
struct ACharacter;
struct USkeletalMeshComponent;
struct APlayerState;

struct APlayerState
{
    uint8_t  pad[Offsets::PLAYER_STATE_NAME];
    FString  PlayerName;
};

struct USkeletalMeshComponent
{
    uint8_t  pad0[Offsets::MESH_COMPONENT_SPACE_BASES];
    uintptr_t ComponentSpaceBases; // TArray<FTransform>
    int32_t  BoneCount;

    FVector GetBoneLocation(int boneIdx) const
    {
        if (!ComponentSpaceBases) return {};
        auto* transforms = reinterpret_cast<FTransform*>(ComponentSpaceBases);
        return transforms[boneIdx].GetBoneLocation();
    }
};

// APawn — pad sizes computed safely using static_assert-friendly constants.
// Raw offset reads are used in feature code; this struct is for documentation.
struct APawn
{
    uint8_t  pad_mesh[Offsets::PAWN_MESH];                    // 0x000
    USkeletalMeshComponent* Mesh;                              // 0x280
    // gap to Health
    uint8_t  pad_health[Offsets::PAWN_HEALTH
                        - Offsets::PAWN_MESH
                        - sizeof(void*)];                      // 0x288
    float    Health;                                           // 0x1B8
    // gap to TeamId
    uint8_t  pad_team[Offsets::PAWN_TEAM_ID
                      - Offsets::PAWN_HEALTH
                      - sizeof(float)];                        // 0x1BC
    int32_t  TeamId;                                           // 0x3C8
    // gap to PlayerState
    uint8_t  pad_ps[Offsets::PAWN_PLAYER_STATE
                    - Offsets::PAWN_TEAM_ID
                    - sizeof(int32_t)];                        // 0x3CC
    APlayerState* PlayerState;                                 // 0x2B8
};

struct APlayerController
{
    uint8_t   pad[0x440];
    FRotator  ControlRotation;
};

struct UGameInstance
{
    uint8_t              pad[Offsets::GAME_INSTANCE_LOCAL_PLAYERS];
    TArray<uintptr_t>    LocalPlayers;
};

struct UWorld
{
    uint8_t         pad[Offsets::UWORLD_GAME_INSTANCE];
    UGameInstance*  GameInstance;
    uint8_t         pad2[Offsets::UWORLD_GAME_STATE
                         - Offsets::UWORLD_GAME_INSTANCE
                         - sizeof(void*)];
    AGameStateBase* GameState;
};
