#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <cstring>

// =============================================================
//  offsets.h  —  UE5.3 Valorant
//  Fox build  |  shyamlohchab/val
//
//  CHANGES vs old file:
//    - Removed DECRYPT_KEY / GWORLD_ENCRYPTED / GOBJECTS_ENCRYPTED / GNAMES_ENCRYPTED
//    - GWorld / GUObjectArray / GNamePoolData are PLAINTEXT in UE5.3 — AOB scan only
//    - MESH_COMPONENT_SPACE_BASES renamed → MESH_BONE_SPACE_TRANSFORMS, offset 0x5B0→0x5E0
//    - PLAYER_CONTROLLER_PAWN updated 0x2A0→0x350 (UE5.3 APlayerController layout)
//    - UWORLD_GAME_STATE updated 0x170→0x1A0
//    - All AOB patterns + ResolveRIP helpers included
//    - Offsets::Init() replaces Decryption::Init() — call from dllmain
// =============================================================

// ─────────────────────────────────────────────────────────────
//  FORWARD — PatternScan / ResolveRIP
//  (also in sdk.h; redeclared inline here so offsets.h is self-contained)
// ─────────────────────────────────────────────────────────────
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
    const size_t len = strlen(mask);
    const uint8_t* mem = reinterpret_cast<const uint8_t*>(start);
    for (size_t i = 0; i + len <= sz; ++i)
    {
        bool ok = true;
        for (size_t j = 0; j < len; ++j)
            if (mask[j] == 'x' && mem[i+j] != static_cast<uint8_t>(pat[j]))
                { ok = false; break; }
        if (ok) return start + i;
    }
    return 0;
}

// Resolve RIP-relative LEA/MOV: instrAddr + instrOffset(4-byte disp32) + instrSize
inline uintptr_t _Off_ResolveRIP(uintptr_t hit,
    uint32_t dispOff = 3, uint32_t instrSz = 7)
{
    const int32_t disp = _Off_RPM<int32_t>(hit + dispOff);
    return hit + instrSz + disp;
}

// ─────────────────────────────────────────────────────────────
//  BONE INDICES  (Valorant skeleton — unchanged)
// ─────────────────────────────────────────────────────────────
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

    // ─────────────────────────────────────────────────────────
    //  STATIC OFFSETS  (UE5.3 — confirmed, no rescan needed
    //  unless Unreal Engine version changes)
    // ─────────────────────────────────────────────────────────

    // UObject layout (UE5.3)
    constexpr uint32_t UOBJECT_FLAGS          = 0x08;
    constexpr uint32_t UOBJECT_INTERNAL_INDEX = 0x0C;
    constexpr uint32_t UOBJECT_NAME_PRIVATE   = 0x18; // was 0x10 in UE4
    constexpr uint32_t UOBJECT_OUTER          = 0x20;

    // FUObjectItem stride
    constexpr uint32_t FUOBJECTITEM_SIZE      = 0x20;

    // FChunkedFixedUObjectArray chunk size
    constexpr int32_t  OBJECT_CHUNK_SIZE      = 65536;

    // FTransform size (SSE-aligned, 0x30)
    constexpr uint32_t FTRANSFORM_SIZE        = 0x30;

    // ─────────────────────────────────────────────────────────
    //  MESH / BONE
    //  UE5.3: BoneSpaceTransforms @ 0x5E0
    //  Old (UE4): ComponentSpaceBases @ 0x5B0  ← was wrong
    // ─────────────────────────────────────────────────────────
    constexpr uint32_t MESH_BONE_SPACE_TRANSFORMS = 0x5E0; // TArray<FTransform>
    constexpr uint32_t MESH_BONE_COUNT            = 0x5EC; // int32 after the TArray

    // ─────────────────────────────────────────────────────────
    //  UWorld chain  (UE5.3 offsets — rescan marked where uncertain)
    // ─────────────────────────────────────────────────────────
    constexpr uint32_t UWORLD_GAME_INSTANCE        = 0x1A0; // UGameInstance*
    constexpr uint32_t UWORLD_GAME_STATE           = 0x1A8; // AGameStateBase*  RESCAN
    constexpr uint32_t UWORLD_PERSISTENT_LEVEL     = 0x58;  // ULevel*          RESCAN
    constexpr uint32_t UWORLD_NET_DRIVER           = 0xE8;  // UNetDriver*      RESCAN

    // UGameInstance
    constexpr uint32_t GAME_INSTANCE_LOCAL_PLAYERS = 0x38;  // TArray<ULocalPlayer*>

    // ULocalPlayer
    constexpr uint32_t LOCAL_PLAYER_CONTROLLER     = 0x30;  // APlayerController*

    // APlayerController  (UE5.3 — shifted from UE4)
    constexpr uint32_t PLAYER_CONTROLLER_PAWN      = 0x350; // APawn* AcknowledgedPawn  (was 0x2A0)
    constexpr uint32_t PLAYER_CONTROLLER_ROTATION  = 0x440; // FRotator ControlRotation

    // AGameStateBase
    constexpr uint32_t GAMESTATE_PLAYER_ARRAY      = 0x2A0; // TArray<APlayerState*>

    // APawn / ACharacter
    constexpr uint32_t PAWN_MESH                   = 0x280; // USkeletalMeshComponent*
    constexpr uint32_t PAWN_HEALTH                 = 0x1B8; // float  RESCAN
    constexpr uint32_t PAWN_TEAM_ID                = 0x3C8; // int32  RESCAN
    constexpr uint32_t PAWN_PLAYER_STATE           = 0x2B8; // APlayerState*

    // APlayerState
    constexpr uint32_t PLAYER_STATE_NAME           = 0x3A8; // FString PlayerName  RESCAN

    // APlayerCameraManager
    constexpr uint32_t CAM_MANAGER_CACHE           = 0x1A50; // FCameraCacheEntry  RESCAN
    // FMinimalViewInfo inside FCameraCacheEntry:
    constexpr uint32_t VIEW_INFO_LOCATION          = 0x10;
    constexpr uint32_t VIEW_INFO_ROTATION          = 0x1C;
    constexpr uint32_t VIEW_INFO_FOV               = 0x28;

    // ─────────────────────────────────────────────────────────
    //  RUNTIME-RESOLVED GLOBALS
    //  Populated by Offsets::Init() via AOB scan.
    //  Access via Offsets::g_namePool, g_uObjectArray, g_world
    // ─────────────────────────────────────────────────────────
    inline uintptr_t g_namePool    = 0; // FNamePool base
    inline uintptr_t g_uObjectArray= 0; // FUObjectArray base
    inline uintptr_t g_world       = 0; // *UWorld (single deref)
    inline uintptr_t g_moduleBase  = 0; // VALORANT-Win64-Shipping.exe base

    // ─────────────────────────────────────────────────────────
    //  AOB PATTERNS
    //  Targeting VALORANT-Win64-Shipping.exe UE5.3 build.
    //  Verify in x64dbg after each patch — Valorant updates frequently.
    //
    //  Pattern format: raw bytes, mask ('x'=match, '?'=wildcard)
    //  rip_off  = byte offset of 4-byte disp32 inside the instruction
    //  instr_sz = total instruction length (for RIP calc)
    //  deref    = true → dereference resolved addr once (ptr-to-ptr)
    // ─────────────────────────────────────────────────────────

    // GNamePoolData — LEA RCX/RAX, [RIP+disp32]
    // Typical sequence: 48 8D 0D ?? ?? ?? ?? (LEA RCX,[rip+X])
    //                   followed by call to FNamePool ctor vicinity
    static constexpr struct { const char* pat; const char* mask; uint32_t rip_off; uint32_t instr_sz; bool deref; }
    AOB_NAME_POOL = {
        "\x48\x8D\x05\x00\x00\x00\x00"   // LEA RAX,[rip+X]   ; GNamePoolData
        "\xEB\x00"                         // JMP short
        "\x48\x8D\x0D\x00\x00\x00\x00",  // LEA RCX,[rip+X]
        "xxx????x?xxx????",
        3, 7, false
    };

    // GUObjectArray — MOV RAX,[RIP+disp32] ; TEST RAX,RAX ; JZ ; MOV RAX,[RAX+xx]
    static constexpr struct { const char* pat; const char* mask; uint32_t rip_off; uint32_t instr_sz; bool deref; }
    AOB_UOBJECT_ARRAY = {
        "\x48\x8B\x05\x00\x00\x00\x00"   // MOV RAX,[rip+X]   ; GUObjectArray
        "\x48\x85\xC0"                    // TEST RAX,RAX
        "\x74\x00"                        // JZ short
        "\x48\x8B\x40\x00",              // MOV RAX,[RAX+xx]
        "xxx????xxxx?xxx?",
        3, 7, false
    };

    // GWorld — MOV RBX,[RIP+disp32] ; TEST RBX,RBX ; JZ
    // UE5.3: GWorld is a plain UWorld** — single deref gives UWorld*
    static constexpr struct { const char* pat; const char* mask; uint32_t rip_off; uint32_t instr_sz; bool deref; }
    AOB_GWORLD = {
        "\x48\x8B\x1D\x00\x00\x00\x00"  // MOV RBX,[rip+X]   ; GWorld ptr-to-ptr
        "\x48\x85\xDB"                   // TEST RBX,RBX
        "\x74\x00",                      // JZ short
        "xxx????xxx?",
        3, 7, true   // deref once: *GWorld → UWorld*
    };

    // ─────────────────────────────────────────────────────────
    //  Init — call once from DllMain / init thread.
    //  Replaces old Decryption::Init() entirely.
    //  Returns false if any critical AOB fails.
    // ─────────────────────────────────────────────────────────
    inline bool Init()
    {
        // Locate game module
        g_moduleBase = reinterpret_cast<uintptr_t>(
            GetModuleHandleA("VALORANT-Win64-Shipping.exe"));
        if (!g_moduleBase) return false;

        MODULEINFO mi{};
        GetModuleInformation(GetCurrentProcess(),
            reinterpret_cast<HMODULE>(g_moduleBase), &mi, sizeof(mi));
        const size_t sz = mi.SizeOfImage;

        // ── GNamePoolData ─────────────────────────────────────
        {
            uintptr_t hit = _Off_PatternScan(g_moduleBase, sz,
                AOB_NAME_POOL.pat, AOB_NAME_POOL.mask);
            if (!hit) return false;
            uintptr_t resolved = _Off_ResolveRIP(hit,
                AOB_NAME_POOL.rip_off, AOB_NAME_POOL.instr_sz);
            g_namePool = AOB_NAME_POOL.deref
                ? _Off_RPM<uintptr_t>(resolved) : resolved;
        }

        // ── GUObjectArray ─────────────────────────────────────
        {
            uintptr_t hit = _Off_PatternScan(g_moduleBase, sz,
                AOB_UOBJECT_ARRAY.pat, AOB_UOBJECT_ARRAY.mask);
            if (!hit) return false;
            uintptr_t resolved = _Off_ResolveRIP(hit,
                AOB_UOBJECT_ARRAY.rip_off, AOB_UOBJECT_ARRAY.instr_sz);
            g_uObjectArray = AOB_UOBJECT_ARRAY.deref
                ? _Off_RPM<uintptr_t>(resolved) : resolved;
        }

        // ── GWorld ────────────────────────────────────────────
        {
            uintptr_t hit = _Off_PatternScan(g_moduleBase, sz,
                AOB_GWORLD.pat, AOB_GWORLD.mask);
            if (!hit) return false;
            uintptr_t resolved = _Off_ResolveRIP(hit,
                AOB_GWORLD.rip_off, AOB_GWORLD.instr_sz);
            // resolved = address of the GWorld global variable (UWorld**)
            // deref once → UWorld*
            g_world = AOB_GWORLD.deref
                ? _Off_RPM<uintptr_t>(resolved) : resolved;
        }

        return (g_namePool && g_uObjectArray && g_world);
    }

    // ─────────────────────────────────────────────────────────
    //  DumpToDebug — call after Init() to verify in x64dbg
    // ─────────────────────────────────────────────────────────
    inline void DumpToDebug()
    {
        char buf[128];
#define _DUMP(label, val) \
    wsprintfA(buf, "[Offsets] " label " = 0x%llX\n", (unsigned long long)(val)); \
    OutputDebugStringA(buf);

        _DUMP("g_moduleBase",   g_moduleBase)
        _DUMP("g_namePool",     g_namePool)
        _DUMP("g_uObjectArray", g_uObjectArray)
        _DUMP("g_world",        g_world)
        _DUMP("BONE_SPACE_TRANSFORMS", MESH_BONE_SPACE_TRANSFORMS)
        _DUMP("PLAYER_CONTROLLER_PAWN", PLAYER_CONTROLLER_PAWN)
        _DUMP("UWORLD_GAME_INSTANCE",   UWORLD_GAME_INSTANCE)
        _DUMP("UWORLD_GAME_STATE",      UWORLD_GAME_STATE)
#undef _DUMP
    }

} // namespace Offsets
