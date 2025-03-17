#include "App.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "implot.h"

// Global font pointers that can be accessed from TradingUI
ImFont* g_defaultFont = nullptr;
ImFont* g_boldFont = nullptr;
ImFont* g_mediumFont = nullptr;
ImFont* g_smallFont = nullptr;

App::App() {
    m_ui = std::make_unique<TradingUI>();
}

App::~App() {
    Shutdown();
}

bool App::Initialize(HWND hwnd) {
    m_hwnd = hwnd;

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        return false;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // LOAD FONTS ONLY HERE - CENTRALIZED FONT LOADING
    g_defaultFont = io.Fonts->AddFontDefault();

    // Try to load the custom font
    ImFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.PixelSnapH = true;

    // Load font.ttf with different sizes for different use cases
    g_boldFont = io.Fonts->AddFontFromFileTTF("fonts/font.ttf", 16.0f, &config);
    if (g_boldFont == nullptr) {
        OutputDebugStringA("Warning: Failed to load font.ttf for bold font\n");
        g_boldFont = g_defaultFont;
    }

    g_mediumFont = io.Fonts->AddFontFromFileTTF("fonts/font.ttf", 14.0f, &config);
    if (g_mediumFont == nullptr) {
        OutputDebugStringA("Warning: Failed to load font.ttf for medium font\n");
        g_mediumFont = g_defaultFont;
    }

    g_smallFont = io.Fonts->AddFontFromFileTTF("fonts/font.ttf", 12.0f, &config);
    if (g_smallFont == nullptr) {
        OutputDebugStringA("Warning: Failed to load font.ttf for small font\n");
        g_smallFont = g_defaultFont;
    }

    // Setup ImPlot context
    ImPlot::CreateContext();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(m_device, m_deviceContext);

    // Initialize UI
    m_ui->Initialize();

    m_initialized = true;
    return true;
}

void App::Shutdown() {
    if (!m_initialized)
        return;

    // ImGui cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    // ImPlot cleanup
    ImPlot::DestroyContext();

    ImGui::DestroyContext();

    // DirectX cleanup
    CleanupDeviceD3D();

    m_initialized = false;
}

bool App::Run() {
    if (!m_initialized)
        return false;

    // Handle window being minimized
    if (m_swapChainOccluded && m_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
        ::Sleep(10);
        return true;
    }
    m_swapChainOccluded = false;

    // Handle window resize
    if (m_resizeWidth != 0 && m_resizeHeight != 0) {
        CleanupRenderTarget();
        m_swapChain->ResizeBuffers(0, m_resizeWidth, m_resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        m_resizeWidth = m_resizeHeight = 0;
        CreateRenderTarget();
    }

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Render UI
    m_ui->Render();

    // Rendering
    ImGui::Render();
    const float clear_color_with_alpha[4] = {
        m_clearColor.x * m_clearColor.w,
        m_clearColor.y * m_clearColor.w,
        m_clearColor.z * m_clearColor.w,
        m_clearColor.w
    };
    m_deviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
    m_deviceContext->ClearRenderTargetView(m_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Present
    HRESULT hr = m_swapChain->Present(1, 0);
    m_swapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

    return true;
}

void App::HandleResize(UINT width, UINT height) {
    m_resizeWidth = width;
    m_resizeHeight = height;
}

bool App::CreateDeviceD3D(HWND hWnd) {
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, &featureLevel, &m_deviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, &featureLevel, &m_deviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void App::CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (m_swapChain) { m_swapChain->Release(); m_swapChain = nullptr; }
    if (m_deviceContext) { m_deviceContext->Release(); m_deviceContext = nullptr; }
    if (m_device) { m_device->Release(); m_device = nullptr; }
}

void App::CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    m_device->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
    pBackBuffer->Release();
}

void App::CleanupRenderTarget() {
    if (m_mainRenderTargetView) { m_mainRenderTargetView->Release(); m_mainRenderTargetView = nullptr; }
}