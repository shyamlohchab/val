#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <cstring>

// ============================================================
//  offsets.h  —  UE5.3 Valorant  (VALORANT-Win64-Shipping.exe)
//  Fox build  |  shyamlohchab/val
//
//  UE5.3 CHANGES vs old UE4 build:
//    - NO decrypt chain: GWorld / GUObjectArray / GNamePool
//      are plaintext globals — AOB scan + RIP resolve only.
//    - DECRYPT_KEY / GWORLD_ENCRYPTED / GOBJECTS_ENCRYPTED /
//      GNAMES_ENCRYPTED are GONE. decryption.cpp is dead code.
//    - UObject::NamePrivate moved +0x10 → +0x18 (8-byte vtable
//      extension in UE5 ObjectFlags/InternalIndex layout).
//    - FUObjectItem stride: 0x18 (UE4) → 0x20 (UE5).
//    - USkeletal BoneSpaceTransforms @ 0x5E0
//      (was ComponentSpaceBases @ 0x5B0 in UE4).
//    - APlayerController::AcknowledgedPawn @ 0x350
//      (was 0x2A0 in UE4 — UE5.3 AActor/AController padding grew).
//    - AGameStateBase::PlayerArray @ 0x2A0
//      (was 0x170 in UE4).
//    - APlayerCameraManager cache @ 0x1A50
//      (was ~0x440 in UE4 — rescan if binary updates).
//    - Offsets::Init() replaces Decryption::Init() entirely.
//      Call once from DllMain / init thread.
// ============================================================

// ────────────────────────────────────────────────────────────
//  INTERNAL HELPERS  (self-contained, no sdk.h dependency)
// ────────────────────────────────────────────────────────────

// Lightweight RPM wrapper used only inside offsets.h
template<typename T>
static inline T _Off_RPM(uintptr_t addr)
{
    T v{};
    ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(addr),
        &v, sizeof(T), nullptr);
    return v;
}

// Classic byte-pattern scanner (x = match, ? = wildcard)
inline uintptr_t _Off_PatternScan(uintptr_t start, size_t sz,
    const char* pat, const char* mask)
{
    const size_t  len = strlen(mask);
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

// ────────────────────────────────────────────────────────────
//  BONE INDICES  (Valorant skeleton — unchanged across patches)
// ────────────────────────────────────────────────────────────
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

    // ────────────────────────────────────────────────────────
    //  STATIC OFFSETS  (UE5.3 confirmed)
    //  Rescan in x64dbg / ReClass after each Valorant patch.
    // ────────────────────────────────────────────────────────

    // ── UObject layout (UE5.3) ──────────────────────────────
    // +0x00  vtable          (8 bytes)
    // +0x08  ObjectFlags     (int32)
    // +0x0C  InternalIndex   (int32)
    // +0x10  pad             (8 bytes)  ← UE5 added here
    // +0x18  NamePrivate     (FName, 8 bytes)
    // +0x20  Outer           (UObject*)
    constexpr uint32_t UOBJECT_FLAGS          = 0x08;
    constexpr uint32_t UOBJECT_INTERNAL_INDEX = 0x0C;
    constexpr uint32_t UOBJECT_NAME_PRIVATE   = 0x18; // was 0x10 in UE4
    constexpr uint32_t UOBJECT_OUTER          = 0x20;

    // ── FUObjectItem stride (UE5.3 = 0x20, was 0x18 in UE4) ─
    constexpr uint32_t FUOBJECTITEM_SIZE      = 0x20;

    // ── FChunkedFixedUObjectArray chunk size ─────────────────
    constexpr int32_t  OBJECT_CHUNK_SIZE      = 65536;

    // ── FTransform size (SSE-aligned, 0x30 = 48 bytes) ───────
    constexpr uint32_t FTRANSFORM_SIZE        = 0x30;

    // ── USkeletal mesh / bones ───────────────────────────────
    //  UE5.3: BoneSpaceTransforms  @ 0x5E0  (TArray<FTransform>)
    //  UE4  : ComponentSpaceBases  @ 0x5B0  (was wrong for UE5)
    constexpr uint32_t MESH_BONE_SPACE_TRANSFORMS = 0x5E0;
    // Num field sits inside the TArray header (+0x08 from Data ptr)
    constexpr uint32_t MESH_BONE_COUNT            = 0x5E8;

    // ── UWorld chain ─────────────────────────────────────────
    //  GWorld is a plaintext UWorld** in UE5.3.
    //  Single deref → UWorld*.
    //
    //  UWorld offsets (rescan after binary updates):
    constexpr uint32_t UWORLD_PERSISTENT_LEVEL    = 0x58;  // ULevel*
    constexpr uint32_t UWORLD_NET_DRIVER           = 0xE8;  // UNetDriver*
    constexpr uint32_t UWORLD_GAME_INSTANCE        = 0x1A0; // UGameInstance*
    constexpr uint32_t UWORLD_GAME_STATE           = 0x1A8; // AGameStateBase*

    // ── UGameInstance ────────────────────────────────────────
    constexpr uint32_t GAME_INSTANCE_LOCAL_PLAYERS = 0x38;  // TArray<ULocalPlayer*>

    // ── ULocalPlayer ─────────────────────────────────────────
    constexpr uint32_t LOCAL_PLAYER_CONTROLLER     = 0x30;  // APlayerController*

    // ── APlayerController (UE5.3) ────────────────────────────
    //  UE5.3 AController layout grew due to extra UE5 fields
    //  in AActor (+0x30 shift vs UE4 APlayerController).
    //  AcknowledgedPawn is the locally confirmed pawn pointer.
    constexpr uint32_t PLAYER_CONTROLLER_PAWN      = 0x350; // APawn* AcknowledgedPawn (was 0x2A0)
    constexpr uint32_t PLAYER_CONTROLLER_ROTATION  = 0x440; // FRotator ControlRotation

    // ── AGameStateBase ───────────────────────────────────────
    //  PlayerArray grew with UE5 AActor padding.
    constexpr uint32_t GAMESTATE_PLAYER_ARRAY       = 0x2A0; // TArray<APlayerState*>

    // ── APawn / ACharacter ───────────────────────────────────
    //  Mesh: UE5 APawn has extra fields before MeshComponent.
    constexpr uint32_t PAWN_MESH                    = 0x280; // USkeletalMeshComponent*
    constexpr uint32_t PAWN_PLAYER_STATE            = 0x2B8; // APlayerState*
    constexpr uint32_t PAWN_HEALTH                  = 0x1B8; // float  (rescan — game-specific)
    constexpr uint32_t PAWN_TEAM_ID                 = 0x3C8; // int32  (rescan — game-specific)

    // ── APlayerState ─────────────────────────────────────────
    constexpr uint32_t PLAYER_STATE_NAME            = 0x3A8; // FString PlayerName (rescan)

    // ── APlayerCameraManager ─────────────────────────────────
    //  FCameraCacheEntry grew significantly in UE5.
    //  Rescan if camera is broken after a patch.
    constexpr uint32_t CAM_MANAGER_CACHE            = 0x1A50; // FCameraCacheEntry

    // ── FMinimalViewInfo inside FCameraCacheEntry ─────────────
    constexpr uint32_t VIEW_INFO_LOCATION            = 0x10;  // FVector
    constexpr uint32_t VIEW_INFO_ROTATION            = 0x1C;  // FRotator
    constexpr uint32_t VIEW_INFO_FOV                 = 0x28;  // float

    // ────────────────────────────────────────────────────────
    //  RUNTIME-RESOLVED GLOBALS
    //  Populated by Offsets::Init() via AOB scan.
    //  All three are plaintext in UE5.3 — no decrypt needed.
    // ────────────────────────────────────────────────────────
    inline uintptr_t g_namePool    = 0; // FNamePool base (GNamePoolData)
    inline uintptr_t g_uObjectArray= 0; // FUObjectArray base (GUObjectArray)
    inline uintptr_t g_world       = 0; // UWorld* (single deref of GWorld)
    inline uintptr_t g_moduleBase  = 0; // VALORANT-Win64-Shipping.exe image base

    // ────────────────────────────────────────────────────────
    //  AOB PATTERNS
    //  Target: VALORANT-Win64-Shipping.exe  UE5.3 build.
    //  Verify in x64dbg after each Valorant patch.
    //
    //  Format:
    //    pat      — raw bytes (\x00 for wildcard slots)
    //    mask     — 'x' = match, '?' = wildcard
    //    rip_off  — byte offset of the 4-byte disp32 inside instruction
    //    instr_sz — total instruction length (for RIP calc)
    //    deref    — true → dereference resolved addr once (ptr-to-ptr)
    // ────────────────────────────────────────────────────────

    // GNamePoolData
    //  48 8D 05 ?? ?? ?? ??   LEA RAX,[rip+X]   ; GNamePoolData
    //  EB 00                  JMP short
    //  48 8D 0D ?? ?? ?? ??   LEA RCX,[rip+X]
    //
    //  We match the second LEA (RCX variant) which is more stable.
    //  rip_off=3, instr_sz=7 → standard RIP-relative LEA.
    static constexpr struct { const char* pat; const char* mask; uint32_t rip_off; uint32_t instr_sz; bool deref; }
    AOB_NAME_POOL = {
        "\x48\x8D\x05\x00\x00\x00\x00"   // LEA RAX,[rip+X]  ; GNamePoolData
        "\xEB\x00"                         // JMP short
        "\x48\x8D\x0D\x00\x00\x00\x00",  // LEA RCX,[rip+X]
        "xxx????x?xxx????",
        // We want the FIRST LEA (RAX variant) — rip_off=3, instr_sz=7
        3, 7, false
    };

    // GUObjectArray
    //  48 8B 05 ?? ?? ?? ??   MOV RAX,[rip+X]   ; GUObjectArray
    //  48 85 C0               TEST RAX,RAX
    //  74 ??                  JZ short
    //  48 8B 40 ??            MOV RAX,[RAX+xx]
    static constexpr struct { const char* pat; const char* mask; uint32_t rip_off; uint32_t instr_sz; bool deref; }
    AOB_UOBJECT_ARRAY = {
        "\x48\x8B\x05\x00\x00\x00\x00"   // MOV RAX,[rip+X]   ; GUObjectArray
        "\x48\x85\xC0"                    // TEST RAX,RAX
        "\x74\x00"                        // JZ short
        "\x48\x8B\x40\x00",              // MOV RAX,[RAX+xx]
        "xxx????xxxx?xxx?",
        3, 7, false
    };

    // GWorld
    //  48 8B 1D ?? ?? ?? ??   MOV RBX,[rip+X]   ; GWorld (UWorld**)
    //  48 85 DB               TEST RBX,RBX
    //  74 ??                  JZ short
    //
    //  deref=true: resolved addr is the GWorld global (UWorld**).
    //  Single RPM dereference gives us the live UWorld*.
    static constexpr struct { const char* pat; const char* mask; uint32_t rip_off; uint32_t instr_sz; bool deref; }
    AOB_GWORLD = {
        "\x48\x8B\x1D\x00\x00\x00\x00"  // MOV RBX,[rip+X]   ; GWorld ptr-to-ptr
        "\x48\x85\xDB"                   // TEST RBX,RBX
        "\x74\x00",                      // JZ short
        "xxx????xxxx?",
        3, 7, true   // deref once: *GWorld → UWorld*
    };

    // ────────────────────────────────────────────────────────
    //  Init()
    //  Call ONCE from DllMain / init thread.
    //  Replaces old Decryption::Init() entirely.
    //  Returns false if any critical AOB fails.
    // ────────────────────────────────────────────────────────
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

        // ── GNamePoolData ────────────────────────────────────
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

        // ── GWorld ───────────────────────────────────────────
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

    // ────────────────────────────────────────────────────────
    //  DumpToDebug  — call after Init() to verify in x64dbg
    // ────────────────────────────────────────────────────────
    inline void DumpToDebug()
    {
        char buf[256];
#define _DUMP(label, val) \
    wsprintfA(buf, "[Offsets] " label " = 0x%llX\n", (unsigned long long)(val)); \
    OutputDebugStringA(buf);

        _DUMP("g_moduleBase",          g_moduleBase)
        _DUMP("g_namePool",            g_namePool)
        _DUMP("g_uObjectArray",        g_uObjectArray)
        _DUMP("g_world",               g_world)
        _DUMP("BONE_SPACE_TRANSFORMS", MESH_BONE_SPACE_TRANSFORMS)
        _DUMP("PLAYER_CONTROLLER_PAWN",PLAYER_CONTROLLER_PAWN)
        _DUMP("UWORLD_GAME_INSTANCE",  UWORLD_GAME_INSTANCE)
        _DUMP("UWORLD_GAME_STATE",     UWORLD_GAME_STATE)
        _DUMP("GAMESTATE_PLAYER_ARRAY",GAMESTATE_PLAYER_ARRAY)
        _DUMP("PAWN_MESH",             PAWN_MESH)
        _DUMP("CAM_MANAGER_CACHE",     CAM_MANAGER_CACHE)
#undef _DUMP
    }

} // namespace Offsets
