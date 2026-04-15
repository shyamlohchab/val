# ValorantExternal

Proper C++ project structure rebuilt from source dump.

## Structure

```
ValorantExternal/
  src/
    dllmain.cpp              — entry point, init sequence
    decryption.h / .cpp      — GWorld/GObjects/GNames decryption
    offsets.h                — all patch-specific offsets (update per patch)
    sdk.h                    — UE4 type wrappers (FVector, TArray, etc.)
    math.h                   — W2S, angle math, smoothing
    chams.h                  — chams config + interface
    menu.h / dx12hook.h / wndproc.h  — hook headers
    wndproc.cpp              — WndProc hook for ImGui input
    hkPostRender.cpp         — hooked Present, ImGui init + render
    ManualMapInjector.cpp    — manual map injector (no LoadLibrary)
    features/
      aimbot.h / .cpp        — aimbot + triggerbot
      esp.h / .cpp           — player ESP (boxes, skeleton, names, health)
      chams.cpp              — chams material override
      skinchanger.h / .cpp   — client-side skin changer
    game/
      GameClasses.h          — UWorld, APawn, APlayerController wrappers
      UCanvas.cpp            — ImGui draw helpers
      UAresInventory.cpp     — inventory/equippable component access
    ui/
      ImGuiMenu.cpp          — ImGui tabbed menu
```

## Build

1. Open `ValorantExternal.sln` in Visual Studio 2022
2. Set platform to **x64**, config to **Release**
3. Place ImGui headers under `ValorantExternal/include/imgui/`
4. Build — outputs `ValorantExternal.dll` to `bin/Release/`
5. Inject with the included manual map injector or any external injector

## Offsets

All offsets are in `src/offsets.h`. Update `GWORLD_ENCRYPTED`,
`GOBJECTS_ENCRYPTED`, `GNAMES_ENCRYPTED`, and `DECRYPT_KEY` after each patch.
Use IDA Pro or Ghidra on `ShooterGame-Win64-Shipping.exe`.

## Dependencies

- ImGui (docking or master branch) — place in `ValorantExternal/include/imgui/`
- DirectX 12 SDK (included with Windows SDK 10.0+)
- Visual Studio 2022 with C++20 toolset
