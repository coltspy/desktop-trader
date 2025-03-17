#include "App.h"
#include <string>  // For std::string and std::stof
#include <random>  // For random price simulation

App::App() {
    // Initialize trading state values
    strcpy(m_tradingState.priceBuf, "1850.45");
    strcpy(m_tradingState.amountBuf, "0.5");
    strcpy(m_tradingState.stopPriceBuf, "1840.00");

    // Initialize animation values
    m_animationState.displayedPrice = 1850.45f;
    m_animationState.targetPrice = 1850.45f;
    m_animationState.priceChangeTime = 0.0f;
    m_animationState.lastUpdateTime = 0.0f;
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
        // Trading Menu
        if (ImGui::BeginMenu("Trading")) {
            if (ImGui::MenuItem("Trade History")) {}
            if (ImGui::MenuItem("Account Settings")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Connect Exchange API")) {}
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
            ImGui::EndMenu();
        }

        // Settings Menu - simple settings options
        if (ImGui::BeginMenu("Settings")) {
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
            if (ImGui::MenuItem("Show Order Book", nullptr, &m_menuState.showOrderBook)) {}
            if (ImGui::MenuItem("Show Depth Chart", nullptr, &m_menuState.showDepthChart)) {}
            ImGui::EndMenu();
        }

        // Help Menu
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Documentation")) {}
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

    // Show demo window if enabled (for development only)
    if (m_menuState.showDemo) {
        ImGui::ShowDemoWindow(&m_menuState.showDemo);
    }
}

void App::RenderChartWindow() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Chart", nullptr, flags);

    // Set padding for better appearance
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));

    // Chart title
    ImGui::Text("ETH/USD Price Chart");

    // Placeholder for actual chart
    ImVec2 chartSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 40);
    ImGui::GetWindowDrawList()->AddRect(
        ImGui::GetCursorScreenPos(),
        ImVec2(ImGui::GetCursorScreenPos().x + chartSize.x, ImGui::GetCursorScreenPos().y + chartSize.y),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.5f, 0.5f, 0.5f))
    );

    ImGui::GetWindowDrawList()->AddText(
        ImVec2(ImGui::GetCursorScreenPos().x + chartSize.x / 2 - 100, ImGui::GetCursorScreenPos().y + chartSize.y / 2),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.5f, 0.5f, 0.8f)),
        "Chart visualization will be implemented here"
    );

    ImGui::PopStyleVar();
    ImGui::End();
}

void App::RenderPositionsWindow() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Positions", nullptr, flags);

    // Custom styling for this window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 6.0f));

    // Positions title
    ImGui::Text("Open Positions");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Create a table for positions
    if (ImGui::BeginTable("PositionsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Setup headers
        ImGui::TableSetupColumn("Symbol");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Entry Price");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableSetupColumn("Current Price");
        ImGui::TableSetupColumn("P/L");
        ImGui::TableHeadersRow();

        // Sample position data
        const float profitLoss = m_animationState.displayedPrice - 1795.23f;
        const float plPercent = (profitLoss / 1795.23f) * 100.0f;

        // Row 1
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::Text("ETH/USD");

        ImGui::TableNextColumn();
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), "Long");

        ImGui::TableNextColumn();
        ImGui::Text("$1,795.23");

        ImGui::TableNextColumn();
        ImGui::Text("0.75 ETH");

        ImGui::TableNextColumn();
        ImGui::Text("$%.2f", m_animationState.displayedPrice);

        ImGui::TableNextColumn();
        if (profitLoss >= 0) {
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), "+$%.2f (%.1f%%)", profitLoss * 0.75f, plPercent);
        }
        else {
            ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "-$%.2f (%.1f%%)", -profitLoss * 0.75f, -plPercent);
        }

        // Row 2 - another sample position
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::Text("BTC/USD");

        ImGui::TableNextColumn();
        ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Short");

        ImGui::TableNextColumn();
        ImGui::Text("$52,450.00");

        ImGui::TableNextColumn();
        ImGui::Text("0.05 BTC");

        ImGui::TableNextColumn();
        ImGui::Text("$52,150.75");

        ImGui::TableNextColumn();
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), "+$14.96 (0.57%)");

        ImGui::EndTable();
    }

    ImGui::PopStyleVar(2);
    ImGui::End();
}

void App::RenderTradingWindow() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Trading", nullptr, flags);

    // Set custom styles for this window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 12.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 6.0f));

    // Draw a header with market name and current price
    const float headerHeight = 60.0f;
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    // Header background
    ImVec2 headerMin = windowPos;
    ImVec2 headerMax = ImVec2(windowPos.x + windowSize.x, windowPos.y + headerHeight);
    ImGui::GetWindowDrawList()->AddRectFilled(headerMin, headerMax,
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.12f, 0.14f, 0.17f, 1.0f)), 0.0f);

    // Market info
    ImGui::SetCursorPosY(10.0f);
    ImGui::SetCursorPosX(16.0f);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font for now
    ImGui::Text("ETH/USD");

    // Update price animation
    float currentTime = ImGui::GetTime();
    if (currentTime > m_animationState.priceChangeTime) {
        m_animationState.priceChangeTime = currentTime + 2.0f + (rand() % 3);
        m_animationState.targetPrice = 1840.0f + (rand() % 2000) / 100.0f;
    }

    // Smooth animation of price
    m_animationState.displayedPrice = m_animationState.displayedPrice * 0.9f + m_animationState.targetPrice * 0.1f;

    // Display price with color based on price movement
    ImGui::SameLine(ImGui::GetWindowWidth() - 120.0f);
    ImVec4 priceColor = (m_animationState.targetPrice > m_animationState.displayedPrice) ?
        ImVec4(0.0f, 0.8f, 0.4f, 1.0f) : ImVec4(0.9f, 0.3f, 0.3f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, priceColor);
    ImGui::Text("$%.2f", m_animationState.displayedPrice);
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::SetCursorPosY(headerHeight + 20.0f);

    // Buy/Sell tabs with improved styling
    const float tabHeight = 50.0f;
    ImVec2 buyTabMin = ImVec2(windowPos.x + 16.0f, windowPos.y + headerHeight + 20.0f);
    ImVec2 buyTabMax = ImVec2(windowPos.x + windowSize.x / 2.0f - 4.0f, buyTabMin.y + tabHeight);
    ImVec2 sellTabMin = ImVec2(windowPos.x + windowSize.x / 2.0f + 4.0f, buyTabMin.y);
    ImVec2 sellTabMax = ImVec2(windowPos.x + windowSize.x - 16.0f, buyTabMin.y + tabHeight);

    // Buy tab
    ImGui::SetCursorPos(ImVec2(16.0f, headerHeight + 20.0f));
    ImGui::BeginGroup();

    ImVec4 buyTabColor = m_tradingState.buySelected ?
        ImVec4(0.0f, 0.7f, 0.4f, 1.0f) : ImVec4(0.2f, 0.2f, 0.25f, 0.7f);
    ImGui::GetWindowDrawList()->AddRectFilled(buyTabMin, buyTabMax,
        ImGui::ColorConvertFloat4ToU32(buyTabColor), 8.0f);

    if (ImGui::InvisibleButton("BuyTab", ImVec2((windowSize.x / 2.0f) - 20.0f, tabHeight))) {
        m_tradingState.buySelected = true;
    }

    // Center the text in the tab
    float textWidth = ImGui::CalcTextSize("BUY").x;
    ImGui::SetCursorPos(ImVec2(windowSize.x / 4.0f - textWidth / 2.0f, headerHeight + 35.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Text("BUY");
    ImGui::PopStyleColor();
    ImGui::EndGroup();

    // Sell tab
    ImGui::SetCursorPos(ImVec2(windowSize.x / 2.0f + 4.0f, headerHeight + 20.0f));
    ImGui::BeginGroup();

    ImVec4 sellTabColor = !m_tradingState.buySelected ?
        ImVec4(0.9f, 0.3f, 0.3f, 1.0f) : ImVec4(0.2f, 0.2f, 0.25f, 0.7f);
    ImGui::GetWindowDrawList()->AddRectFilled(sellTabMin, sellTabMax,
        ImGui::ColorConvertFloat4ToU32(sellTabColor), 8.0f);

    if (ImGui::InvisibleButton("SellTab", ImVec2((windowSize.x / 2.0f) - 20.0f, tabHeight))) {
        m_tradingState.buySelected = false;
    }

    // Center the text in the tab
    textWidth = ImGui::CalcTextSize("SELL").x;
    ImGui::SetCursorPos(ImVec2(windowSize.x * 3.0f / 4.0f - textWidth / 2.0f, headerHeight + 35.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Text("SELL");
    ImGui::PopStyleColor();
    ImGui::EndGroup();

    ImGui::SetCursorPosY(headerHeight + 20.0f + tabHeight + 20.0f);
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    // Order Type with improved styling
    ImGui::Text("Order Type");
    ImGui::SetCursorPosX(16.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.20f, 0.20f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.25f, 0.25f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.15f, 0.15f, 0.20f, 1.0f));

    const char* orderTypes[] = { "Market", "Limit", "Stop", "Stop Limit" };
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 16.0f);
    if (ImGui::Combo("##OrderType", &m_tradingState.orderType, orderTypes, IM_ARRAYSIZE(orderTypes))) {
        // Handle order type change
        if (m_tradingState.orderType == 0) { // Market
            strcpy(m_tradingState.priceBuf, "Market");
        }
        else {
            // Restore price value for non-market orders
            sprintf(m_tradingState.priceBuf, "%.2f", m_tradingState.price);
        }
    }
    ImGui::PopStyleColor(4);

    ImGui::Spacing();
    ImGui::Spacing();

    // Price Input with improved styling
    if (m_tradingState.orderType != 0) { // Not Market order
        ImGui::Text("Price (USD)");
        ImGui::SetCursorPosX(16.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 16.0f);

        // Draw a dollar sign inside the input field
        float inputPosX = ImGui::GetCursorPosX();
        float inputPosY = ImGui::GetCursorPosY();

        if (ImGui::InputText("##Price", m_tradingState.priceBuf, sizeof(m_tradingState.priceBuf))) {
            // Update price value when text changes
            try {
                m_tradingState.price = std::stof(m_tradingState.priceBuf);
            }
            catch (...) {
                // Handle invalid input
            }
        }

        // Draw currency symbol at the right side of the input
        ImVec2 textSize = ImGui::CalcTextSize(m_tradingState.priceBuf);
        ImGui::SameLine(0, 0);
        ImGui::SetCursorPosX(inputPosX + ImGui::GetItemRectSize().x - 30);
        ImGui::SetCursorPosY(inputPosY + 2);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "USD");

        ImGui::PopStyleColor(2);
        ImGui::Spacing();
        ImGui::Spacing();
    }

    // Stop Price with improved styling
    if (m_tradingState.orderType == 2 || m_tradingState.orderType == 3) { // Stop or Stop Limit
        ImGui::Text("Stop Price (USD)");
        ImGui::SetCursorPosX(16.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 16.0f);

        float inputPosX = ImGui::GetCursorPosX();
        float inputPosY = ImGui::GetCursorPosY();

        if (ImGui::InputText("##StopPrice", m_tradingState.stopPriceBuf, sizeof(m_tradingState.stopPriceBuf))) {
            // Update stop price value when text changes
            try {
                m_tradingState.stopPrice = std::stof(m_tradingState.stopPriceBuf);
            }
            catch (...) {
                // Handle invalid input
            }
        }

        // Draw currency symbol at the right side of the input
        ImGui::SameLine(0, 0);
        ImGui::SetCursorPosX(inputPosX + ImGui::GetItemRectSize().x - 30);
        ImGui::SetCursorPosY(inputPosY + 2);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "USD");

        ImGui::PopStyleColor(2);
        ImGui::Spacing();
        ImGui::Spacing();
    }

    // Amount Input with improved styling
    ImGui::Text("Amount (ETH)");
    ImGui::SetCursorPosX(16.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 16.0f);

    float inputPosX = ImGui::GetCursorPosX();
    float inputPosY = ImGui::GetCursorPosY();

    if (ImGui::InputText("##Amount", m_tradingState.amountBuf, sizeof(m_tradingState.amountBuf))) {
        // Update amount value when text changes
        try {
            m_tradingState.amount = std::stof(m_tradingState.amountBuf);
        }
        catch (...) {
            // Handle invalid input
        }
    }

    // Draw currency symbol at the right side of the input
    ImGui::SameLine(0, 0);
    ImGui::SetCursorPosX(inputPosX + ImGui::GetItemRectSize().x - 30);
    ImGui::SetCursorPosY(inputPosY + 2);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "ETH");

    ImGui::PopStyleColor(2);

    // Available Balance Display with improved styling
    ImVec4 textColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    if (m_tradingState.buySelected) {
        ImGui::Text("Available: $%.2f", m_tradingState.availableQuote);
    }
    else {
        ImGui::Text("Available: %.4f ETH", m_tradingState.availableBase);
    }
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // Amount Slider with improved styling
    ImGui::SetCursorPosX(16.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, m_tradingState.buySelected ?
        ImVec4(0.0f, 0.7f, 0.4f, 1.0f) : ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, m_tradingState.buySelected ?
        ImVec4(0.0f, 0.8f, 0.5f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f));

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 16.0f);
    if (ImGui::SliderFloat("##AmountSlider", &m_tradingState.amountPercent, 0.0f, 100.0f, "%.0f%%")) {
        // Update amount based on percentage of available balance
        if (m_tradingState.buySelected) {
            float maxAmount = m_tradingState.availableQuote / m_tradingState.price;
            m_tradingState.amount = maxAmount * (m_tradingState.amountPercent / 100.0f);
        }
        else {
            m_tradingState.amount = m_tradingState.availableBase * (m_tradingState.amountPercent / 100.0f);
        }
        sprintf(m_tradingState.amountBuf, "%.4f", m_tradingState.amount);
    }
    ImGui::PopStyleColor(3);

    // Percentage Buttons with improved styling
    ImGui::SetCursorPosX(16.0f);
    float buttonWidth = (ImGui::GetContentRegionAvail().x - 12.0f) / 4.0f;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.30f, 1.0f));

    auto updateAmountFromPercentage = [this](float percentage) {
        m_tradingState.amountPercent = percentage;
        if (m_tradingState.buySelected) {
            float maxAmount = m_tradingState.availableQuote / m_tradingState.price;
            m_tradingState.amount = maxAmount * (percentage / 100.0f);
        }
        else {
            m_tradingState.amount = m_tradingState.availableBase * (percentage / 100.0f);
        }
        sprintf(m_tradingState.amountBuf, "%.4f", m_tradingState.amount);
        };

    if (ImGui::Button("25%", ImVec2(buttonWidth, 30.0f))) {
        updateAmountFromPercentage(25.0f);
    }

    ImGui::SameLine();
    if (ImGui::Button("50%", ImVec2(buttonWidth, 30.0f))) {
        updateAmountFromPercentage(50.0f);
    }

    ImGui::SameLine();
    if (ImGui::Button("75%", ImVec2(buttonWidth, 30.0f))) {
        updateAmountFromPercentage(75.0f);
    }

    ImGui::SameLine();
    if (ImGui::Button("100%", ImVec2(buttonWidth, 30.0f))) {
        updateAmountFromPercentage(100.0f);
    }

    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // Summary box with shadow effect
    ImVec2 summaryMin = ImVec2(windowPos.x + 16.0f, ImGui::GetCursorScreenPos().y);
    ImVec2 summaryMax = ImVec2(windowPos.x + windowSize.x - 16.0f, summaryMin.y + 80.0f);

    // Shadow effect
    ImGui::GetWindowDrawList()->AddRectFilled(
        ImVec2(summaryMin.x + 5, summaryMin.y + 5),
        ImVec2(summaryMax.x + 5, summaryMax.y + 5),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.3f)), 8.0f);

    // Summary box
    ImGui::GetWindowDrawList()->AddRectFilled(summaryMin, summaryMax,
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.15f, 0.15f, 0.20f, 1.0f)), 8.0f);

    // Calculated total
    float total = m_tradingState.orderType == 0
        ? m_tradingState.amount * m_animationState.displayedPrice // Use current displayed price for market orders
        : m_tradingState.amount * m_tradingState.price;
    float fee = total * 0.002f; // 0.2% fee

    // Summary text
    ImGui::SetCursorPosX(26.0f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 16.0f);
    ImGui::Text("Total:");
    ImGui::SameLine(windowSize.x - 120.0f);
    ImGui::Text("$%.2f", total);

    ImGui::SetCursorPosX(26.0f);
    ImGui::Text("Fee (0.2%%):");
    ImGui::SameLine(windowSize.x - 120.0f);
    ImGui::Text("$%.2f", fee);

    ImGui::SetCursorPosX(26.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::Text("Total + Fee:");
    ImGui::SameLine(windowSize.x - 120.0f);
    ImGui::Text("$%.2f", total + fee);
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // Execute button - green for buy, red for sell
    ImGui::SetCursorPosX(16.0f);

    ImVec4 buttonColor = m_tradingState.buySelected ?
        ImVec4(0.0f, 0.6f, 0.3f, 1.0f) : ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
    ImVec4 buttonHoveredColor = m_tradingState.buySelected ?
        ImVec4(0.0f, 0.7f, 0.4f, 1.0f) : ImVec4(0.9f, 0.3f, 0.3f, 1.0f);
    ImVec4 buttonActiveColor = m_tradingState.buySelected ?
        ImVec4(0.0f, 0.8f, 0.5f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActiveColor);

    std::string buttonText = (m_tradingState.buySelected ? "BUY " : "SELL ") + std::string("ETH");
    if (m_tradingState.orderType == 0) {
        buttonText += " at Market";
    }

    if (ImGui::Button(buttonText.c_str(), ImVec2(ImGui::GetContentRegionAvail().x - 16.0f, 50.0f))) {
        // Execute order - add visual feedback here 
        // For demo, we'll just flash the button by changing its color briefly
    }

    ImGui::PopStyleColor(3);

    // Pop all pushed style variables
    ImGui::PopStyleVar(4);

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