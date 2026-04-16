#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <cstring>

// ============================================================================
//  offsets.h  —  UE5.3 Valorant  (VALORANT-Win64-Shipping.exe)
//  Fox build  |  shyamlohchab/val
//
//  UE5.3 — no decrypt chain. All globals are plaintext.
//  AOB scan + RIP resolve only. Decryption::Init() is dead.
// ============================================================================

// ────────────────────────────────────────────────────────────────────────────
//  INTERNAL HELPERS
// ────────────────────────────────────────────────────────────────────────────

template<typename T>
static inline T _Off_RPM(uintptr_t addr)
{
    T v{};
    ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(addr),
        &v, sizeof(T), nullptr);
    return v;
}

inline uintptr_t _Off_PatternScan(uintptr_t start, size_t sz,
    const char* pat, const char* mask)
{
    const size_t   len = strlen(mask);
    const uint8_t* mem = reinterpret_cast<const uint8_t*>(start);
    for (size_t i = 0; i + len <= sz; ++i)
    {
        bool ok = true;
        for (size_t j = 0; j < len; ++j)
            if (mask[j] == 'x' && mem[i + j] != static_cast<uint8_t>(pat[j]))
            {
                ok = false; break;
            }
        if (ok) return start + i;
    }
    return 0;
}

// Resolve RIP-relative LEA/MOV:  hit + disp32(at dispOff) + instrSz
inline uintptr_t _Off_ResolveRIP(uintptr_t hit,
    uint32_t dispOff = 3, uint32_t instrSz = 7)
{
    const int32_t disp = _Off_RPM<int32_t>(hit + dispOff);
    return hit + instrSz + disp;
}

// ────────────────────────────────────────────────────────────────────────────
//  BONE INDICES  (Valorant skeleton — stable across patches)
// ────────────────────────────────────────────────────────────────────────────
namespace Offsets
{
    constexpr int BONE_HEAD   = 8;
    constexpr int BONE_NECK   = 7;
    constexpr int BONE_CHEST  = 5;
    constexpr int BONE_PELVIS = 0;
    constexpr int BONE_LHAND  = 32;
    constexpr int BONE_RHAND  = 62;
    constexpr int BONE_LFOOT  = 78;
    constexpr int BONE_RFOOT  = 98;

    // ────────────────────────────────────────────────────────────────────────
    //  LATEST PATCH — MODULE-RELATIVE OFFSETS
    //  All values are relative to VALORANT-Win64-Shipping.exe base.
    //  Usage: g_moduleBase + OFFSET_XXX
    // ────────────────────────────────────────────────────────────────────────

    // ──  Core engine globals ─────────────────────────────────────────────────
    constexpr uintptr_t OFFSET_GWORLD              = 0x0C1AD2A0; // UWorld**
    constexpr uintptr_t OFFSET_FNAME_POOL          = 0x0C34B800; // FNamePool (GNamePoolData)

    // ──  UObject system ──────────────────────────────────────────────────────
    constexpr uintptr_t OFFSET_PROCESS_EVENT       = 0x01B74230; // UObject::ProcessEvent
    constexpr uintptr_t OFFSET_STATIC_FIND_OBJECT  = 0x01BA05C0; // StaticFindObject
    constexpr uintptr_t OFFSET_STATIC_LOAD_OBJECT  = 0x01BA40F0; // StaticLoadObject

    // ──  Memory ──────────────────────────────────────────────────────────────
    constexpr uintptr_t OFFSET_FMEMORY_MALLOC      = 0x01747C10; // FMemory::Malloc

    // ──  Rendering / mesh ────────────────────────────────────────────────────
    constexpr uintptr_t OFFSET_BONE_MATRIX         = 0x040EBC10; // GetBoneMatrix / bone cache fn
    constexpr uintptr_t OFFSET_SET_OUTLINE_MODE    = 0x0407F040; // SetOutlineMode

    // ──  Exception / VEH trigger ─────────────────────────────────────────────
    constexpr uintptr_t OFFSET_TRIGGER_VEH         = 0x0177F496; // TriggerVeh

    // ──  Game-specific functions ──────────────────────────────────────────────
    constexpr uintptr_t OFFSET_PLAY_FINISHER                  = 0x062424D0;
    constexpr uintptr_t OFFSET_GET_SPREAD_VALUES              = 0x062DEAA0;
    constexpr uintptr_t OFFSET_GET_SPREAD_ANGLES              = 0x06F04280;
    constexpr uintptr_t OFFSET_TOVECTOR_AND_NORMALIZE         = 0x01847290;
    constexpr uintptr_t OFFSET_TOANGLE_AND_NORMALIZE          = 0x01841A90;
    constexpr uintptr_t OFFSET_GET_FIRING_LOCATION_AND_DIR   = 0x06AA7A30;

    // ────────────────────────────────────────────────────────────────────────
    //  STATIC STRUCT OFFSETS  (UE5.3 — rescan after engine bump)
    // ────────────────────────────────────────────────────────────────────────

    // UObject layout (UE5.3)
    // +0x00  vtable
    // +0x08  ObjectFlags     (int32)
    // +0x0C  InternalIndex   (int32)
    // +0x10  pad             (8 bytes) ← UE5 added
    // +0x18  NamePrivate     (FName)
    // +0x20  Outer           (UObject*)
    constexpr uint32_t UOBJECT_FLAGS          = 0x08;
    constexpr uint32_t UOBJECT_INTERNAL_INDEX = 0x0C;
    constexpr uint32_t UOBJECT_NAME_PRIVATE   = 0x18; // was 0x10 in UE4
    constexpr uint32_t UOBJECT_OUTER          = 0x20;

    // FUObjectItem stride (UE5.3 = 0x20, was 0x18 in UE4)
    constexpr uint32_t FUOBJECTITEM_SIZE      = 0x20;

    // FChunkedFixedUObjectArray chunk size
    constexpr int32_t  OBJECT_CHUNK_SIZE      = 65536;

    // FTransform size (SSE-aligned, 48 bytes)
    constexpr uint32_t FTRANSFORM_SIZE        = 0x30;

    // USkeletal mesh — BoneSpaceTransforms (UE5.3)
    // Was ComponentSpaceBases @ 0x5B0 in UE4
    constexpr uint32_t MESH_BONE_SPACE_TRANSFORMS = 0x5E0; // TArray<FTransform>
    constexpr uint32_t MESH_BONE_COUNT            = 0x5E8; // TArray.Num (Data+0x08)

    // UWorld chain
    constexpr uint32_t UWORLD_PERSISTENT_LEVEL    = 0x58;  // ULevel*
    constexpr uint32_t UWORLD_NET_DRIVER          = 0xE8;  // UNetDriver*
    constexpr uint32_t UWORLD_GAME_INSTANCE       = 0x1A0; // UGameInstance*
    constexpr uint32_t UWORLD_GAME_STATE          = 0x1A8; // AGameStateBase*

    // UGameInstance
    constexpr uint32_t GAME_INSTANCE_LOCAL_PLAYERS = 0x38;  // TArray<ULocalPlayer*>

    // ULocalPlayer
    constexpr uint32_t LOCAL_PLAYER_CONTROLLER     = 0x30;  // APlayerController*

    // APlayerController (UE5.3 — +0xB0 shift vs UE4)
    constexpr uint32_t PLAYER_CONTROLLER_PAWN      = 0x350; // APawn* AcknowledgedPawn
    constexpr uint32_t PLAYER_CONTROLLER_ROTATION  = 0x440; // FRotator ControlRotation

    // AGameStateBase
    constexpr uint32_t GAMESTATE_PLAYER_ARRAY       = 0x2A0; // TArray<APlayerState*>

    // APawn / ACharacter
    constexpr uint32_t PAWN_MESH                    = 0x280; // USkeletalMeshComponent*
    constexpr uint32_t PAWN_PLAYER_STATE            = 0x2B8; // APlayerState*
    constexpr uint32_t PAWN_HEALTH                  = 0x1B8; // float  (rescan — game-specific)
    constexpr uint32_t PAWN_TEAM_ID                 = 0x3C8; // int32  (rescan — game-specific)

    // APlayerState
    constexpr uint32_t PLAYER_STATE_NAME            = 0x3A8; // FString PlayerName (rescan)

    // APlayerCameraManager
    constexpr uint32_t CAM_MANAGER_CACHE            = 0x1A50; // FCameraCacheEntry

    // FMinimalViewInfo inside FCameraCacheEntry
    constexpr uint32_t VIEW_INFO_LOCATION           = 0x10;
    constexpr uint32_t VIEW_INFO_ROTATION           = 0x1C;
    constexpr uint32_t VIEW_INFO_FOV                = 0x28;

    // ────────────────────────────────────────────────────────────────────────
    //  RUNTIME-RESOLVED GLOBALS
    //  Populated by Offsets::Init() via AOB scan.
    // ────────────────────────────────────────────────────────────────────────
    inline uintptr_t g_namePool    = 0; // FNamePool base
    inline uintptr_t g_uObjectArray= 0; // FUObjectArray base
    inline uintptr_t g_world       = 0; // UWorld* (single deref)
    inline uintptr_t g_moduleBase  = 0; // image base

    // Resolved function pointers (set in Init)
    inline uintptr_t g_processEvent              = 0;
    inline uintptr_t g_staticFindObject          = 0;
    inline uintptr_t g_staticLoadObject          = 0;
    inline uintptr_t g_fmemoryMalloc             = 0;
    inline uintptr_t g_boneMatrix                = 0;
    inline uintptr_t g_setOutlineMode            = 0;
    inline uintptr_t g_triggerVeh                = 0;
    inline uintptr_t g_playFinisher              = 0;
    inline uintptr_t g_getSpreadValues           = 0;
    inline uintptr_t g_getSpreadAngles           = 0;
    inline uintptr_t g_toVectorAndNormalize      = 0;
    inline uintptr_t g_toAngleAndNormalize       = 0;
    inline uintptr_t g_getFiringLocationAndDir   = 0;

    // ────────────────────────────────────────────────────────────────────────
    //  AOB PATTERNS  (verify in x64dbg after each patch)
    // ────────────────────────────────────────────────────────────────────────

    // GNamePoolData — LEA RAX,[rip+X]
    static constexpr struct { const char* pat; const char* mask; uint32_t rip_off; uint32_t instr_sz; bool deref; }
    AOB_NAME_POOL = {
        "\x48\x8D\x05\x00\x00\x00\x00"
        "\xEB\x00"
        "\x48\x8D\x0D\x00\x00\x00\x00",
        "xxx????x?xxx????",
        3, 7, false
    };

    // GUObjectArray — MOV RAX,[rip+X]
    static constexpr struct { const char* pat; const char* mask; uint32_t rip_off; uint32_t instr_sz; bool deref; }
    AOB_UOBJECT_ARRAY = {
        "\x48\x8B\x05\x00\x00\x00\x00"
        "\x48\x85\xC0"
        "\x74\x00"
        "\x48\x8B\x40\x00",
        "xxx????xxx x?xxx?",
        3, 7, false
    };

    // GWorld — MOV RBX,[rip+X] ; deref once → UWorld*
    static constexpr struct { const char* pat; const char* mask; uint32_t rip_off; uint32_t instr_sz; bool deref; }
    AOB_GWORLD = {
        "\x48\x8B\x1D\x00\x00\x00\x00"
        "\x48\x85\xDB"
        "\x74\x00",
        "xxx????xxxx?",
        3, 7, true
    };

    // ────────────────────────────────────────────────────────────────────────
    //  Init()  —  call once from DllMain / init thread.
    //  Replaces Decryption::Init() entirely.
    //  Returns false if any critical AOB fails.
    // ────────────────────────────────────────────────────────────────────────
    inline bool Init()
    {
        g_moduleBase = reinterpret_cast<uintptr_t>(
            GetModuleHandleA("VALORANT-Win64-Shipping.exe"));
        if (!g_moduleBase) return false;

        MODULEINFO mi{};
        GetModuleInformation(GetCurrentProcess(),
            reinterpret_cast<HMODULE>(g_moduleBase), &mi, sizeof(mi));
        const size_t sz = mi.SizeOfImage;

        // ──  Resolve function pointers from static offsets ──────────────────
        g_processEvent              = g_moduleBase + OFFSET_PROCESS_EVENT;
        g_staticFindObject          = g_moduleBase + OFFSET_STATIC_FIND_OBJECT;
        g_staticLoadObject          = g_moduleBase + OFFSET_STATIC_LOAD_OBJECT;
        g_fmemoryMalloc             = g_moduleBase + OFFSET_FMEMORY_MALLOC;
        g_boneMatrix                = g_moduleBase + OFFSET_BONE_MATRIX;
        g_setOutlineMode            = g_moduleBase + OFFSET_SET_OUTLINE_MODE;
        g_triggerVeh                = g_moduleBase + OFFSET_TRIGGER_VEH;
        g_playFinisher              = g_moduleBase + OFFSET_PLAY_FINISHER;
        g_getSpreadValues           = g_moduleBase + OFFSET_GET_SPREAD_VALUES;
        g_getSpreadAngles           = g_moduleBase + OFFSET_GET_SPREAD_ANGLES;
        g_toVectorAndNormalize      = g_moduleBase + OFFSET_TOVECTOR_AND_NORMALIZE;
        g_toAngleAndNormalize       = g_moduleBase + OFFSET_TOANGLE_AND_NORMALIZE;
        g_getFiringLocationAndDir   = g_moduleBase + OFFSET_GET_FIRING_LOCATION_AND_DIR;

        // ──  GNamePoolData (AOB → RIP resolve) ──────────────────────────────
        {
            uintptr_t hit = _Off_PatternScan(g_moduleBase, sz,
                AOB_NAME_POOL.pat, AOB_NAME_POOL.mask);
            if (!hit) return false;
            uintptr_t resolved = _Off_ResolveRIP(hit,
                AOB_NAME_POOL.rip_off, AOB_NAME_POOL.instr_sz);
            g_namePool = AOB_NAME_POOL.deref
                ? _Off_RPM<uintptr_t>(resolved) : resolved;
        }

        // ──  GUObjectArray (AOB → RIP resolve) ──────────────────────────────
        {
            uintptr_t hit = _Off_PatternScan(g_moduleBase, sz,
                AOB_UOBJECT_ARRAY.pat, AOB_UOBJECT_ARRAY.mask);
            if (!hit) return false;
            uintptr_t resolved = _Off_ResolveRIP(hit,
                AOB_UOBJECT_ARRAY.rip_off, AOB_UOBJECT_ARRAY.instr_sz);
            g_uObjectArray = AOB_UOBJECT_ARRAY.deref
                ? _Off_RPM<uintptr_t>(resolved) : resolved;
        }

        // ──  GWorld (AOB → RIP resolve → single deref) ──────────────────────
        {
            uintptr_t hit = _Off_PatternScan(g_moduleBase, sz,
                AOB_GWORLD.pat, AOB_GWORLD.mask);
            if (!hit) return false;
            uintptr_t resolved = _Off_ResolveRIP(hit,
                AOB_GWORLD.rip_off, AOB_GWORLD.instr_sz);
            g_world = AOB_GWORLD.deref
                ? _Off_RPM<uintptr_t>(resolved) : resolved;
        }

        return (g_namePool && g_uObjectArray && g_world);
    }

    // ────────────────────────────────────────────────────────────────────────
    //  DumpToDebug  — call after Init() to verify in x64dbg
    // ────────────────────────────────────────────────────────────────────────
    inline void DumpToDebug()
    {
        char buf[256];
#define _DUMP(label, val) \
    wsprintfA(buf, "[Offsets] " label " = 0x%llX\n", (unsigned long long)(val)); \
    OutputDebugStringA(buf);

        _DUMP("g_moduleBase",              g_moduleBase)
        _DUMP("g_namePool",                g_namePool)
        _DUMP("g_uObjectArray",            g_uObjectArray)
        _DUMP("g_world",                   g_world)
        _DUMP("g_processEvent",            g_processEvent)
        _DUMP("g_staticFindObject",        g_staticFindObject)
        _DUMP("g_staticLoadObject",        g_staticLoadObject)
        _DUMP("g_fmemoryMalloc",           g_fmemoryMalloc)
        _DUMP("g_boneMatrix",              g_boneMatrix)
        _DUMP("g_setOutlineMode",          g_setOutlineMode)
        _DUMP("g_triggerVeh",              g_triggerVeh)
        _DUMP("g_playFinisher",            g_playFinisher)
        _DUMP("g_getSpreadValues",         g_getSpreadValues)
        _DUMP("g_getSpreadAngles",         g_getSpreadAngles)
        _DUMP("g_toVectorAndNormalize",    g_toVectorAndNormalize)
        _DUMP("g_toAngleAndNormalize",     g_toAngleAndNormalize)
        _DUMP("g_getFiringLocationAndDir", g_getFiringLocationAndDir)
        _DUMP("BONE_SPACE_TRANSFORMS",     MESH_BONE_SPACE_TRANSFORMS)
        _DUMP("PLAYER_CONTROLLER_PAWN",    PLAYER_CONTROLLER_PAWN)
        _DUMP("UWORLD_GAME_INSTANCE",      UWORLD_GAME_INSTANCE)
        _DUMP("UWORLD_GAME_STATE",         UWORLD_GAME_STATE)
        _DUMP("GAMESTATE_PLAYER_ARRAY",    GAMESTATE_PLAYER_ARRAY)
        _DUMP("PAWN_MESH",                 PAWN_MESH)
        _DUMP("CAM_MANAGER_CACHE",         CAM_MANAGER_CACHE)
#undef _DUMP
    }

} // namespace Offsets
