# ValorantExternal

Proper C++ project structure — rebuilt and fixed from source dump.

## What was fixed

- **`dx12hook.cpp`** — added full VMT hook implementation (`Init`/`Shutdown`/`OnPresent`). Previously `Init()` and `Shutdown()` were declared but never defined — build-breaking linker error.
- **`aimbot.cpp`** — fixed local player position (was hardcoded `{}`). Now reads pelvis bone from local pawn chain. Fixed team check (was `(void)team` stub).
- **`ManualMapInjector.h`** — added missing header so the injector is properly includable.
- **`game/UCanvas.h`** — added missing header for the `Draw::` namespace.
- **`game/UAresInventory.h`** — added missing header for the `Inventory::` namespace.
- **`features/skinchanger.h/.cpp`** — `Config` now has `skinMap` + `originalSkinMap`. `Apply()`/`Revert()` are fully wired to `Inventory::` API.
- **`features/chams.cpp`** — proper structure with device caching and correct `visibleOnly` guard.
- **`game/GameClasses.h`** — pad math made safe; `UWorld` layout fixed.
- **`ValorantExternal.vcxproj`** — added `dx12hook.cpp`, `ManualMapInjector.h`, `UCanvas.h`, `UAresInventory.h`, and all feature headers to the project.

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
    dx12hook.cpp             — full VMT hook: Init/Shutdown/OnPresent
    wndproc.cpp              — WndProc hook for ImGui input
    hkPostRender.cpp         — placeholder (logic moved to dx12hook.cpp)
    ManualMapInjector.h/.cpp — manual map injector (no LoadLibrary)
    features/
      aimbot.h / .cpp        — aimbot + triggerbot (local pos fixed)
      esp.h / .cpp           — player ESP (boxes, skeleton, names, health)
      chams.cpp              — chams material override
      skinchanger.h / .cpp   — client-side skin changer (wired to Inventory API)
    game/
      GameClasses.h          — UWorld, APawn, APlayerController wrappers
      UCanvas.h / .cpp       — ImGui draw helpers
      UAresInventory.h / .cpp — inventory/equippable component access
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
