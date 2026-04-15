#pragma once
#include <Windows.h>
#include <cstdint>

namespace Decryption
{
    // Initializes all decrypted pointers (GObjects, GNames, GWorld, etc.)
    bool Init();

    // Returns decrypted GWorld pointer
    uintptr_t GetGWorld();

    // Returns decrypted GObjects base
    uintptr_t GetGObjects();

    // Returns decrypted GNames base
    uintptr_t GetGNames();

    // Decrypts a single encrypted pointer using Valorant's XOR/ROR scheme
    uintptr_t DecryptPointer(uintptr_t encrypted);

    // Reads a decrypted UObject field at given offset
    template<typename T>
    T ReadDecrypted(uintptr_t base, uint32_t offset)
    {
        return *reinterpret_cast<T*>(DecryptPointer(base) + offset);
    }
}
