#include <Windows.h>
#include "sdk.h"
#include "offsets.h"
#include "decryption.h"
#include "menu.h"
#include "dx12hook.h"
#include "wndproc.h"
#include "features/aimbot.h"
#include "features/esp.h"
#include "features/chams.h"
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
    // Wait for the game to fully load
    Sleep(2000);

    // Initialize decryption layer (GObjects, GNames, GWorld)
    if (!Decryption::Init())
    {
        FreeLibraryAndExitThread(hModule, 0);
        return;
    }

    // Hook DX12 Present / PostRender
    if (!DX12Hook::Init())
    {
        FreeLibraryAndExitThread(hModule, 0);
        return;
    }

    // Hook WndProc for ImGui input
    WndProc::Init();

    // Main loop — runs until INSERT is pressed to unload
    while (!GetAsyncKeyState(VK_END))
    {
        Sleep(10);
    }

    // Cleanup
    DX12Hook::Shutdown();
    WndProc::Shutdown();
    FreeLibraryAndExitThread(hModule, 0);
}
