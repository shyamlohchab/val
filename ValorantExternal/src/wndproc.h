#pragma once
#include <Windows.h>

// ============================================================
//  WndProc hook — intercepts window messages for ImGui input
// ============================================================
namespace WndProc
{
    void Init();
    void Shutdown();

    // The hooked WndProc — forwards to ImGui then to original
    LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}
