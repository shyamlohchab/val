#include <Windows.h>
#include "sdk.h"
#include "offsets.h"
#include "decryption.h"
#include "menu.h"
#include "dx12hook.h"
#include "wndproc.h"
#include "features/aimbot.h"
#include "features/esp.h"
#include "chams.h"
#include "features/skinchanger.h"

// Forward declarations
void MainThread(HMODULE hModule);

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread),
            hModule, 0, nullptr);
    }
    return TRUE;
}

void MainThread(HMODULE hModule)
{
    // Wait for game to fully load — increase if AOB scan returns 0
    Sleep(2000);

    // --- INIT LAYER 1: Offsets (AOB scan, RIP resolve, function ptrs) ---
    // Decryption::Init() delegates to Offsets::Init() internally.
    // Correct module: VALORANT-Win64-Shipping.exe
    if (!Decryption::Init())
    {
        // AOB scan failed — patch probably changed a sig.
        // Enable _DEBUG and check Offsets::DumpToDebug() to see which ptr is 0.
        FreeLibraryAndExitThread(hModule, 0);
        return;
    }

    // --- INIT LAYER 2: SDK globals (GNamePool, GUObjectArray, GWorld) ---
    // Wire up sdk.h namespace so FNamePool::GetName() and
    // SDK::GetObjectByIndex() / SDK::GetWorld() work correctly.
    SDK::Init(
        Offsets::g_namePool,       // FNamePool base
        Offsets::g_uObjectArray,   // FUObjectArray base
        Offsets::g_world           // GWorld** (single deref inside GetWorld())
    );

#ifdef _DEBUG
    // Dump all resolved addresses to OutputDebugString.
    // Check in x64dbg View -> Log or DebugView.
    Offsets::DumpToDebug();
#endif

    // --- INIT LAYER 3: Rendering hook ---
    if (!DX12Hook::Init())
    {
        FreeLibraryAndExitThread(hModule, 0);
        return;
    }

    // --- INIT LAYER 4: Input hook ---
    WndProc::Init();

    // Main loop — END key to unload
    while (!GetAsyncKeyState(VK_END))
    {
        Sleep(10);
    }

    // Cleanup
    DX12Hook::Shutdown();
    WndProc::Shutdown();
    FreeLibraryAndExitThread(hModule, 0);
}
