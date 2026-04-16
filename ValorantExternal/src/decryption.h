#pragma once
#include <Windows.h>
#include <cstdint>

namespace Decryption
{
    // UE5.3 — delegates to Offsets::Init() internally.
    // Returns false only if VALORANT-Win64-Shipping.exe isn't loaded
    // or any critical AOB pattern fails to match.
    bool Init();

    // Passthrough in UE5.3 — no XOR/ROR chain.
    // Kept for API compatibility.
    uintptr_t DecryptPointer(uintptr_t encrypted);

    // Plaintext globals — read directly from Offsets:: namespace.
    uintptr_t GetGWorld();
    uintptr_t GetGObjects();
    uintptr_t GetGNames();

    // ReadDecrypted<T> removed — use RPM<T>(addr) from sdk.h instead.
    // DecryptPointer is a passthrough so it was just an RPM wrapper.
}
