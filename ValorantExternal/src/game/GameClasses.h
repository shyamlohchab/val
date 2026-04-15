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
    uint8_t pad[Offsets::PLAYER_STATE_NAME];
    FString PlayerName;
};

struct USkeletalMeshComponent
{
    uint8_t pad0[Offsets::MESH_COMPONENT_SPACE_BASES];
    uintptr_t ComponentSpaceBases; // TArray<FTransform>
    int32_t   BoneCount;

    FVector GetBoneLocation(int boneIdx) const
    {
        if (!ComponentSpaceBases) return {};
        auto* transforms = reinterpret_cast<FTransform*>(ComponentSpaceBases);
        return transforms[boneIdx].GetBoneLocation();
    }
};

struct APawn
{
    uint8_t pad0[Offsets::PAWN_MESH];
    USkeletalMeshComponent* Mesh;
    uint8_t pad1[Offsets::PAWN_HEALTH - Offsets::PAWN_MESH - sizeof(void*)];
    float   Health;
    uint8_t pad2[Offsets::PAWN_TEAM_ID - Offsets::PAWN_HEALTH - sizeof(float)];
    int32_t TeamId;
    uint8_t pad3[Offsets::PAWN_PLAYER_STATE - Offsets::PAWN_TEAM_ID - sizeof(int32_t)];
    APlayerState* PlayerState;
};

struct APlayerController
{
    uint8_t pad[0x440];
    FRotator ControlRotation;
};

struct UGameInstance
{
    uint8_t pad[Offsets::GAME_INSTANCE_LOCAL_PLAYERS];
    TArray<uintptr_t> LocalPlayers;
};

struct UWorld
{
    uint8_t pad[Offsets::UWORLD_GAME_INSTANCE];
    UGameInstance* GameInstance;
    uint8_t pad2[0x28];
    AGameStateBase* GameState;
};
