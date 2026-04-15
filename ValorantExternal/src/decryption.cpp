#include "decryption.h"
#include "offsets.h"
#include <Windows.h>

namespace Decryption
{
    static uintptr_t g_base    = 0;
    static uintptr_t g_gworld  = 0;
    static uintptr_t g_gobjs   = 0;
    static uintptr_t g_gnames  = 0;

    // Valorant pointer decryption stub.
    // The actual XOR/ROR constants change per patch — update from IDA.
    uintptr_t DecryptPointer(uintptr_t encrypted)
    {
        // Example multi-step decryption (patch-specific, update as needed):
        //   mov rax, [rcx+0x60]   ; load encrypted ptr
        //   ror rax, 0x13
        //   xor rax, rbx          ; rbx = some runtime key
        //   sub rax, rdi
        // Stub — replace with current patch decryption:
        return encrypted ^ Offsets::DECRYPT_KEY;
    }

    bool Init()
    {
        g_base = reinterpret_cast<uintptr_t>(GetModuleHandleA("ShooterGame-Win64-Shipping.exe"));
        if (!g_base) return false;

        // Read encrypted pointers from static offsets
        uintptr_t enc_gworld = *reinterpret_cast<uintptr_t*>(g_base + Offsets::GWORLD_ENCRYPTED);
        uintptr_t enc_gobjs  = *reinterpret_cast<uintptr_t*>(g_base + Offsets::GOBJECTS_ENCRYPTED);
        uintptr_t enc_gnames = *reinterpret_cast<uintptr_t*>(g_base + Offsets::GNAMES_ENCRYPTED);

        g_gworld = DecryptPointer(enc_gworld);
        g_gobjs  = DecryptPointer(enc_gobjs);
        g_gnames = DecryptPointer(enc_gnames);

        return (g_gworld && g_gobjs && g_gnames);
    }

    uintptr_t GetGWorld()   { return g_gworld; }
    uintptr_t GetGObjects() { return g_gobjs;  }
    uintptr_t GetGNames()   { return g_gnames; }
}
