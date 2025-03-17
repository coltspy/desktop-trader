#include "App.h"

App::App() {
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigWindowsMoveFromTitleBarOnly = true;              // Move windows only from title bar

    // Setup ImGui style
    SetupImGuiStyle();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(m_device, m_deviceContext);

    // Load a font if needed
    /*
    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 16.0f);
    if (!font) font = io.Fonts->AddFontDefault();
    */

    m_initialized = true;
    return true;
}

void App::Shutdown() {
    if (!m_initialized)
        return;

    // ImGui cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
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

    // Render the menu bar first
    RenderMenuBar();

    // Create a fullscreen window for docking
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    // Get the viewport (adjusting for main menu bar)
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Remove window padding/border
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    // Start the root window
    ImGui::Begin("DockSpaceWindow", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // Create the dockspace
    m_dockspaceId = ImGui::GetID("MainDockSpace");
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode |
        ImGuiDockNodeFlags_NoDockingInCentralNode |
        ImGuiDockNodeFlags_AutoHideTabBar;

    ImGui::DockSpace(m_dockspaceId, ImVec2(0.0f, 0.0f), dockspace_flags);

    // Create the dock layout the first time
    if (m_firstFrame) {
        m_firstFrame = false;
        CreateDockingLayout();
    }

    // Render the windows
    RenderChartWindow();
    RenderPositionsWindow();
    RenderTradingWindow();

    // End the root window
    ImGui::End();

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

void App::SetupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Dark theme
    ImGui::StyleColorsDark();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.13f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.13f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.13f, 0.17f, 1.00f);

    // Specific colors for the menu bar
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);

    // Make window borders nearly invisible
    style.WindowBorderSize = 1.0f;
    style.WindowRounding = 0.0f;
    style.WindowPadding = ImVec2(0.0f, 0.0f);

    // Style adjustments for better UI
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
}

void App::CreateDockingLayout() {
    // Get viewport information
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Reset any existing dockspace
    ImGui::DockBuilderRemoveNode(m_dockspaceId);
    ImGui::DockBuilderAddNode(m_dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(m_dockspaceId, viewport->WorkSize);

    // Create the layout
    ImGuiID dock_main_id = m_dockspaceId;

    // Split right side for trading panel (40% width)
    ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.4f, nullptr, &dock_main_id);

    // Split remaining area for chart on top (70% height) and positions below (30% height)
    ImGuiID dock_chart_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.7f, nullptr, &dock_main_id);
    ImGuiID dock_positions_id = dock_main_id;

    // Apply lock flags to all nodes
    ImGuiDockNode* node_chart = ImGui::DockBuilderGetNode(dock_chart_id);
    ImGuiDockNode* node_positions = ImGui::DockBuilderGetNode(dock_positions_id);
    ImGuiDockNode* node_right = ImGui::DockBuilderGetNode(dock_right_id);

    std::uint32_t lock_flags = ImGuiDockNodeFlags_NoResize |
        ImGuiDockNodeFlags_NoTabBar |
        ImGuiDockNodeFlags_NoDockingOverMe |
        ImGuiDockNodeFlags_NoDockingInCentralNode;

    if (node_chart) {
        node_chart->LocalFlags |= lock_flags;
        node_chart->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar;
    }
    if (node_positions) {
        node_positions->LocalFlags |= lock_flags;
        node_positions->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar;
    }
    if (node_right) {
        node_right->LocalFlags |= lock_flags;
        node_right->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar;
    }

    // Dock windows to specific locations
    ImGui::DockBuilderDockWindow("Chart", dock_chart_id);
    ImGui::DockBuilderDockWindow("Positions", dock_positions_id);
    ImGui::DockBuilderDockWindow("Trading", dock_right_id);

    // Finalize the docking layout
    ImGui::DockBuilderFinish(m_dockspaceId);
}

void App::RenderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        // File Menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Workspace", "Ctrl+N")) {}
            if (ImGui::MenuItem("Open Config...", "Ctrl+O")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Save Layout", "Ctrl+S")) {}
            if (ImGui::MenuItem("Save Layout As...", "Ctrl+Shift+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // Request app to exit
                ::PostQuitMessage(0);
            }
            ImGui::EndMenu();
        }

        // Edit Menu
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false)) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
            if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Preferences...")) {}
            ImGui::EndMenu();
        }

        // View Menu
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Chart Window", nullptr, true)) {}
            if (ImGui::MenuItem("Positions Window", nullptr, true)) {}
            if (ImGui::MenuItem("Trading Window", nullptr, true)) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Order Book", nullptr, &m_menuState.showOrderBook)) {}
            if (ImGui::MenuItem("Depth Chart", nullptr, &m_menuState.showDepthChart)) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Dark Theme", nullptr, &m_menuState.darkTheme)) {
                // Toggle theme
                if (m_menuState.darkTheme) {
                    ImGui::StyleColorsDark();
                }
                else {
                    ImGui::StyleColorsLight();
                }
                SetupImGuiStyle(); // Apply our custom style on top
            }
            ImGui::Separator();
            if (ImGui::MenuItem("ImGui Demo Window", nullptr, &m_menuState.showDemo)) {}
            if (ImGui::MenuItem("ImGui Metrics", nullptr, &m_menuState.showMetrics)) {}
            ImGui::EndMenu();
        }

        // Trading Menu
        if (ImGui::BeginMenu("Trading")) {
            if (ImGui::MenuItem("Place Buy Order", "Ctrl+B")) {}
            if (ImGui::MenuItem("Place Sell Order", "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Cancel All Orders")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Trading History")) {}
            if (ImGui::MenuItem("Account Settings")) {}
            ImGui::EndMenu();
        }

        // Tools Menu
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Calculator")) {}
            if (ImGui::MenuItem("Screener")) {}
            ImGui::Separator();
            if (ImGui::BeginMenu("Indicators")) {
                if (ImGui::MenuItem("Moving Average")) {}
                if (ImGui::MenuItem("RSI")) {}
                if (ImGui::MenuItem("MACD")) {}
                if (ImGui::MenuItem("Bollinger Bands")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Drawing Tools")) {
                if (ImGui::MenuItem("Trend Line")) {}
                if (ImGui::MenuItem("Fibonacci Retracement")) {}
                if (ImGui::MenuItem("Rectangle")) {}
                if (ImGui::MenuItem("Ellipse")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        // Help Menu
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Documentation")) {}
            if (ImGui::MenuItem("Keyboard Shortcuts")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Check for Updates")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("About Trading Platform")) {}
            ImGui::EndMenu();
        }

        // Right-aligned user balance
        float windowWidth = ImGui::GetWindowWidth();
        float balanceWidth = ImGui::CalcTextSize("Balance: $00,000.00").x;
        ImGui::SameLine(windowWidth - balanceWidth - 20);
        ImGui::Text("Balance: $%.2f", m_menuState.userBalance);

        ImGui::EndMainMenuBar();
    }

    // Show demo window if enabled
    if (m_menuState.showDemo) {
        ImGui::ShowDemoWindow(&m_menuState.showDemo);
    }

    // Show metrics window if enabled
    if (m_menuState.showMetrics) {
        ImGui::ShowMetricsWindow(&m_menuState.showMetrics);
    }
}

void App::RenderChartWindow() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Chart", nullptr, flags);
    // Empty for now - will implement chart drawing later
    ImGui::End();
}

void App::RenderPositionsWindow() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Positions", nullptr, flags);
    // Empty for now - will implement positions table later
    ImGui::End();
}

void App::RenderTradingWindow() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Trading", nullptr, flags);
    // Empty for now - will implement trading controls later
    ImGui::End();
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