#pragma once

#include <d3d11.h>
#include <cstdint> // For uint32_t
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"

// Forward declare the window procedure
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class App {
public:
    App();
    ~App();

    bool Initialize(HWND hwnd);
    void Shutdown();
    bool Run();

    // Handling window resize
    void HandleResize(UINT width, UINT height);

    // Declare WndProc as a friend so it can access private members
    friend LRESULT WINAPI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    // Menu state
    struct {
        bool showDemo = false;
        bool showMetrics = false;
        bool darkTheme = true;
        bool showOrderBook = true;
        bool showDepthChart = false;
        float userBalance = 25420.36f;
    } m_menuState;

    // DirectX/ImGui setup
    bool CreateDeviceD3D(HWND hwnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();

    // UI handling
    void SetupImGuiStyle();
    void CreateDockingLayout();
    void RenderMenuBar();
    void RenderChartWindow();
    void RenderPositionsWindow();
    void RenderTradingWindow();

    // DirectX components
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_deviceContext = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;

    // State
    bool m_initialized = false;
    bool m_firstFrame = true;
    bool m_swapChainOccluded = false;
    UINT m_resizeWidth = 0, m_resizeHeight = 0;
    ImGuiID m_dockspaceId = 0;
    ImVec4 m_clearColor = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    HWND m_hwnd = nullptr;
};