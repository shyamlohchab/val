#include "wndproc.h"
#include "menu.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace WndProc
{
    static HWND            g_hWnd     = nullptr;
    static WNDPROC         g_original = nullptr;

    LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // Feed input to ImGui when menu is open
        if (Menu::g_open)
        {
            if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
                return true;
        }

        // Toggle menu with INSERT
        if (msg == WM_KEYDOWN && wParam == VK_INSERT)
        {
            Menu::g_open = !Menu::g_open;
            // Block cursor when menu is open
            if (Menu::g_open)
                ShowCursor(TRUE);
            else
                ShowCursor(FALSE);
        }

        return CallWindowProcW(g_original, hWnd, msg, wParam, lParam);
    }

    void Init()
    {
        g_hWnd     = FindWindowA(nullptr, "VALORANT");
        g_original = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrW(g_hWnd, GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(HookedWndProc)));
    }

    void Shutdown()
    {
        if (g_hWnd && g_original)
            SetWindowLongPtrW(g_hWnd, GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(g_original));
    }
}
