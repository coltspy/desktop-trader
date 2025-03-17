#pragma once

#include <d3d11.h>
#include <cstdint> // For uint32_t
#include <string>  // For std::string
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
        bool darkTheme = true;
        bool showOrderBook = true;
        bool showDepthChart = false;
        float userBalance = 25420.36f;
    } m_menuState;

    // Trading panel state
    struct {
        bool buySelected = true;
        int orderType = 0; // 0=Market, 1=Limit, 2=Stop, 3=Stop Limit
        float amount = 0.5f;
        float price = 1850.45f;
        float stopPrice = 1840.0f;
        float amountPercent = 50.0f;
        float availableBase = 2.5f;      // Available ETH
        float availableQuote = 12000.0f; // Available USD
        char priceBuf[64] = "1850.45";
        char amountBuf[64] = "0.5";
        char stopPriceBuf[64] = "1840.00";
    } m_tradingState;

    // Animation state for UI elements
    struct {
        float displayedPrice;
        float targetPrice;
        float priceChangeTime;
        float lastUpdateTime;
    } m_animationState;

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