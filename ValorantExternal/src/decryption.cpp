#include "decryption.h"
#include "offsets.h"

namespace Decryption
{
    // UE5.3 — no decrypt chain.
    // All globals are plaintext. Offsets::Init() does the
    // AOB scan + RIP resolve for GWorld, GUObjectArray, GNamePool.
    // This file is a thin shim so callers don't need to change.

    bool Init()
    {
        // Offsets::Init() handles:
        //   - GetModuleHandleA("VALORANT-Win64-Shipping.exe")  <- correct name
        //   - AOB scan for GNamePool, GUObjectArray, GWorld
        //   - RIP resolve + optional single deref
        //   - Populates g_namePool, g_uObjectArray, g_world, g_moduleBase
        //   - Resolves all function pointers
        return Offsets::Init();
    }

    // No decryption needed — pointers are plaintext in UE5.3.
    // Kept for API compatibility; just returns the value as-is.
    uintptr_t DecryptPointer(uintptr_t encrypted)
    {
        return encrypted;
    }

    uintptr_t GetGWorld()   { return Offsets::g_world;        }
    uintptr_t GetGObjects() { return Offsets::g_uObjectArray; }
    uintptr_t GetGNames()   { return Offsets::g_namePool;     }
}
