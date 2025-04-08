// src/main.cpp (updated)
#include <Windows.h>
#include <memory>
#include "App.h"
#include "imgui_impl_win32.h"

// ImGui Win32 message handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Application instance
std::unique_ptr<App> g_app;

// Window message processor
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Let ImGui handle its messages first
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        if (g_app) {
            g_app->HandleResize((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT app menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

int main(int, char**)
{
    // Set up window class
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TradingPlatform", nullptr };
    ::RegisterClassExW(&wc);

    // Create main window maximized
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Trading Platform", WS_OVERLAPPEDWINDOW, 0, 0, screenWidth, screenHeight, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize app
    g_app = std::make_unique<App>();
    if (!g_app->Initialize(hwnd))
    {
        g_app->Shutdown();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Display the window
    ::ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    ::UpdateWindow(hwnd);

    // Main application loop
    bool done = false;
    while (!done)
    {
        // Process window messages
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Run app frame
        if (!g_app->Run())
            done = true;
    }

    // Clean up
    g_app->Shutdown();
    g_app.reset();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}