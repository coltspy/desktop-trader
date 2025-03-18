#pragma once

#include <d3d11.h>
#include <cstdint>
#include <memory>
#include <Windows.h>
#include "TradingUI.h"
#include "CryptoAPIClient.h" // Add CryptoAPIClient include

// Forward declare the window procedure
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class App {
public:
    App();
    ~App();

    bool Initialize(HWND hwnd);
    void Shutdown();
    bool Run();
    void HandleResize(UINT width, UINT height);

    // Friend declaration for WndProc
    friend LRESULT WINAPI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    // DirectX setup
    bool CreateDeviceD3D(HWND hwnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();

    // DirectX components
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_deviceContext = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;

    // UI component
    std::unique_ptr<TradingUI> m_ui;

    // API client
    std::shared_ptr<CryptoAPIClient> m_apiClient;

    // Data update timer
    float m_lastUpdateTime = 0.0f;

    // State variables
    bool m_initialized = false;
    bool m_swapChainOccluded = false;
    UINT m_resizeWidth = 0, m_resizeHeight = 0;
    ImVec4 m_clearColor = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    HWND m_hwnd = nullptr;
};