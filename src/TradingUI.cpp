#include "TradingUI.h"
#include "implot.h" // Add ImPlot include
#include "CryptoAPIClient.h" // Add CryptoAPIClient include
#include "Config.h" // Add Config include
#include <random>
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>

extern ImFont* g_defaultFont;
extern ImFont* g_boldFont;
extern ImFont* g_mediumFont;
extern ImFont* g_smallFont;

TradingUI::TradingUI() {
    // Initialize available symbols from Config
    for (int i = 0; i < Config::UI::AVAILABLE_CRYPTOS_COUNT; i++) {
        m_cryptoState.availableSymbols.push_back(Config::UI::AVAILABLE_CRYPTOS[i]);
    }

    // Set default selected symbol
    m_cryptoState.selectedSymbol = Config::UI::DEFAULT_CRYPTO;
}

void TradingUI::Initialize() {
    // Setup ImGui style
    SetupImGuiStyle();

    // Load custom fonts
    LoadFonts();

    // Initialize chart renderer
    m_chartRenderer.Initialize();

    // Set the initial symbol in the chart renderer
    m_chartRenderer.SetSymbol(m_cryptoState.selectedSymbol);
}

void TradingUI::LoadFonts() {
    // Fonts are loaded in App.cpp - just use the global font pointers
    m_defaultFont = g_defaultFont;
    m_boldFont = g_boldFont;
    m_mediumFont = g_mediumFont;
    m_smallFont = g_smallFont;
}

void TradingUI::Render() {
    // Render the menu bar first
    RenderMenuBar();

    // Create a fullscreen window for docking
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    // Get the viewport
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

    // Show ImPlot demo window if enabled
    if (m_menuState.showImPlotDemo) {
        ImPlot::ShowDemoWindow(&m_menuState.showImPlotDemo);
    }

    // Show ImGui demo window if enabled
    if (m_menuState.showDemo) {
        ImGui::ShowDemoWindow(&m_menuState.showDemo);
    }

    // End the root window
    ImGui::End();

    // Animate price display
    float currentTime = ImGui::GetTime();
    float deltaTime = currentTime - m_animationState.lastUpdateTime;
    m_animationState.lastUpdateTime = currentTime;

    // Gradually animate towards target price
    if (m_animationState.displayedPrice != m_animationState.targetPrice) {
        float animationDuration = 0.5f; // Animation duration in seconds
        float t = std::min((currentTime - m_animationState.priceChangeTime) / animationDuration, 1.0f);
        m_animationState.displayedPrice = m_animationState.displayedPrice + (m_animationState.targetPrice - m_animationState.displayedPrice) * t;

        // If we're close enough, snap to the target price
        if (std::abs(m_animationState.displayedPrice - m_animationState.targetPrice) < 0.01f) {
            m_animationState.displayedPrice = m_animationState.targetPrice;
        }
    }
}

void TradingUI::SetupImGuiStyle() {
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

    // Trade panel specific colors
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.2f, 0.4f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.3f, 0.5f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.2f, 1.0f);

    // Dark theme overrides for buy/sell UI
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);

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

void TradingUI::CreateDockingLayout() {
    // Get viewport information
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Reset any existing dockspace
    ImGui::DockBuilderRemoveNode(m_dockspaceId);
    ImGui::DockBuilderAddNode(m_dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(m_dockspaceId, viewport->WorkSize);

    // Create the layout
    ImGuiID dock_main_id = m_dockspaceId;

    // Split right side for trading panel (40% width)
    ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.3f, nullptr, &dock_main_id);

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

void TradingUI::RenderMenuBar() {
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

        // Settings Menu
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

        // Demo Menu
        if (ImGui::BeginMenu("Demo")) {
            ImGui::MenuItem("ImGui Demo", nullptr, &m_menuState.showDemo);
            ImGui::MenuItem("ImPlot Demo", nullptr, &m_menuState.showImPlotDemo);
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
}

void TradingUI::RenderCryptoSelector() {
    // Only render if we have an API client
    if (!m_apiClient) {
        return;
    }

    // Create a combo box for selecting the cryptocurrency
    ImGui::SetNextItemWidth(120);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));

    if (ImGui::BeginCombo("##CryptoSelector", m_cryptoState.selectedSymbol.c_str())) {
        for (const auto& symbol : m_cryptoState.availableSymbols) {
            bool isSelected = (symbol == m_cryptoState.selectedSymbol);
            if (ImGui::Selectable(symbol.c_str(), isSelected)) {
                if (symbol != m_cryptoState.selectedSymbol) {
                    m_cryptoState.selectedSymbol = symbol;
                    m_chartRenderer.SetSymbol(symbol);

                    // Update the price and chart data for the new symbol
                    UpdatePriceData();
                }
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::PopStyleVar();

    // Show loading indicator if we're fetching data
    if (m_cryptoState.isLoading) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Loading...");
    }

    // Show error message if any
    if (!m_cryptoState.errorMessage.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", m_cryptoState.errorMessage.c_str());

        // Clear error after 3 seconds
        static float errorTime = ImGui::GetTime();
        if (ImGui::GetTime() - errorTime > 3.0f) {
            m_cryptoState.errorMessage.clear();
        }
    }
}

void TradingUI::UpdatePriceData() {
    if (!m_apiClient) {
        m_cryptoState.errorMessage = "API client not initialized";
        return;
    }

    // Set loading state
    m_cryptoState.isLoading = true;
    m_cryptoState.errorMessage.clear();

    // Fetch the latest quote for the selected symbol
    m_apiClient->FetchLatestQuote(m_cryptoState.selectedSymbol, [this](const PriceData& priceData, bool isRealData) {
        // Update the trading state with the new price
        m_tradingState.price = priceData.price;

        // Update whether we're using real API data
        m_cryptoState.usingRealApiData = isRealData;

        // Update the animation state
        m_animationState.targetPrice = priceData.price;
        m_animationState.priceChangeTime = ImGui::GetTime();

        // Format the price string
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << priceData.price;
        strncpy(m_tradingState.priceBuf, ss.str().c_str(), IM_ARRAYSIZE(m_tradingState.priceBuf));

        // Clear loading state
        m_cryptoState.isLoading = false;
        });

    // Fetch historical data for the chart
    m_apiClient->FetchHistoricalData(m_cryptoState.selectedSymbol, [this](const std::vector<PriceData>& historicalData) {
        // Update the chart renderer with the new data
        if (!historicalData.empty()) {
            std::vector<double> timestamps;
            std::vector<double> opens;
            std::vector<double> highs;
            std::vector<double> lows;
            std::vector<double> closes;
            std::vector<double> volumes;

            for (const auto& data : historicalData) {
                timestamps.push_back(data.timestamp);
                opens.push_back(data.open);
                highs.push_back(data.high);
                lows.push_back(data.low);
                closes.push_back(data.close);
                volumes.push_back(data.volume);
            }

            // Update the chart renderer's data
            m_chartRenderer.SetChartData(timestamps, opens, highs, lows, closes, volumes);
        }
        });
}

void TradingUI::RenderChartWindow() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Chart", nullptr, flags);

    // Add some padding for better appearance
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));

    // Use bold font for the title
    ImGui::PushFont(m_boldFont);

    // Add cryptocurrency selector dropdown
    RenderCryptoSelector();

    ImGui::SameLine();
    ImGui::Text("Price Chart");

    // Display current price with animation
    ImGui::SameLine(ImGui::GetWindowWidth() - 250);

    // API status indicator
    if (m_cryptoState.usingRealApiData) {
        ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.0f, 1.0f), "LIVE");
        ImGui::SameLine();
    }
    else {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.0f, 1.0f), "MOCK");
        ImGui::SameLine();
    }

    ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), "$%.2f", m_animationState.displayedPrice);

    ImGui::PopFont();

    ImGui::Spacing();

    // Use the ChartRenderer to render charts (takes the full remaining space)
    m_chartRenderer.RenderCharts();

    ImGui::PopStyleVar();
    ImGui::End();
}

void TradingUI::RenderPositionsWindow() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Positions", nullptr, flags);

    // Custom styling for this window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 6.0f));

    // Positions title with bold font
    ImGui::PushFont(m_boldFont);
    ImGui::Text("Open Positions");
    ImGui::PopFont();

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
        ImGui::Text("%s/USD", m_cryptoState.selectedSymbol.c_str());

        ImGui::TableNextColumn();
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), "Long");

        ImGui::TableNextColumn();
        ImGui::Text("$1,795.23");

        ImGui::TableNextColumn();
        ImGui::Text("0.75 %s", m_cryptoState.selectedSymbol.c_str());

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

void TradingUI::RenderTradingWindow() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Trading", nullptr, flags);

    // Set custom styles for this window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));

    // Dark theme colors
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.08f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.16f, 0.16f, 0.16f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.20f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.28f, 1.00f));

    // Buy/Sell tabs as full-width buttons
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    float halfWidth = (windowSize.x - 16.0f) / 2.0f;

    // Use bold font for buy/sell tabs
    ImGui::PushFont(m_boldFont);

    // Display the current trading pair in the header
    ImGui::Text("%s/USD", m_cryptoState.selectedSymbol.c_str());
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, m_tradingState.buySelected ? ImVec4(0.15f, 0.15f, 0.15f, 1.0f) : ImVec4(0.15f, 0.15f, 0.15f, 0.5f));
    if (ImGui::Button("Buy", ImVec2(halfWidth, 36))) {
        m_tradingState.buySelected = true;
    }

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, !m_tradingState.buySelected ? ImVec4(0.7f, 0.2f, 0.2f, 1.0f) : ImVec4(0.15f, 0.15f, 0.15f, 0.5f));
    if (ImGui::Button("Sell", ImVec2(halfWidth, 36))) {
        m_tradingState.buySelected = false;
    }
    ImGui::PopStyleColor(2);
    ImGui::PopFont();

    ImGui::Spacing();

    // Order type tabs as custom tabs
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 1.0f));

    ImGui::PushFont(m_mediumFont);
    const char* orderTypeLabels[] = { "Market", "Limit" };
    for (int i = 0; i < 2; i++) {
        bool isSelected = (m_tradingState.orderType == i);
        ImGui::PushStyleColor(ImGuiCol_Button, isSelected ? ImVec4(0.8f, 0.2f, 0.2f, 1.0f) : ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, isSelected ? ImVec4(0.9f, 0.3f, 0.3f, 1.0f) : ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, isSelected ? ImVec4(0.8f, 0.2f, 0.2f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, isSelected ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

        float tabWidth = i == 0 ? windowSize.x / 2.0f : windowSize.x / 2.0f;
        if (ImGui::Button(orderTypeLabels[i], ImVec2(tabWidth, 0))) {
            m_tradingState.orderType = i;
        }
        ImGui::PopStyleColor(4);

        if (i < 1) ImGui::SameLine();
    }
    ImGui::PopFont();

    ImGui::PopStyleVar(2);

    // Info text on the right
    ImGui::SameLine(ImGui::GetWindowWidth() - 130);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "1 GIGA San");

    ImGui::Spacing();
    ImGui::Spacing();

    // Amount input field
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 10.0f));
    ImGui::PushFont(m_mediumFont);
    ImGui::Text("AMOUNT");
    ImGui::PopFont();

    ImGui::SameLine(ImGui::GetWindowWidth() - 50);
    ImGui::Text("0");
    ImGui::SameLine(ImGui::GetWindowWidth() - 30);
    ImGui::Text("%%");

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 1.00f));
    ImGui::InputText("##AmountInput", m_tradingState.amountBuf, IM_ARRAYSIZE(m_tradingState.amountBuf));
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    // Percentage buttons
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 8.0f));
    float buttonWidth = (ImGui::GetContentRegionAvail().x - 12.0f) / 4.0f;

    ImGui::PushFont(m_mediumFont);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

    if (ImGui::Button("25 %", ImVec2(buttonWidth, 0))) {
        m_tradingState.amountPercent = 25.0f;
        UpdateAmountFromPercentage(0.25f);
    }

    ImGui::SameLine();
    if (ImGui::Button("50 %", ImVec2(buttonWidth, 0))) {
        m_tradingState.amountPercent = 50.0f;
        UpdateAmountFromPercentage(0.5f);
    }

    ImGui::SameLine();
    if (ImGui::Button("75 %", ImVec2(buttonWidth, 0))) {
        m_tradingState.amountPercent = 75.0f;
        UpdateAmountFromPercentage(0.75f);
    }

    ImGui::SameLine();
    if (ImGui::Button("100 %", ImVec2(buttonWidth, 0))) {
        m_tradingState.amountPercent = 100.0f;
        UpdateAmountFromPercentage(1.0f);
    }
    ImGui::PopFont();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    ImGui::Spacing();
    ImGui::Spacing();

    // Gas section
    ImGui::PushFont(m_smallFont);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Gas -");
    ImGui::SameLine();

    // Gas settings (simplified)
    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "0.002");
    ImGui::SameLine(ImGui::GetWindowWidth() - 180);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "100%%");
    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "0.00100");
    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Spacing();

    // Main action button
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 12.0f));

    ImGui::PushFont(m_boldFont);
    if (m_tradingState.buySelected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
        if (ImGui::Button("Buy", ImVec2(-1, 45))) {
            // Handle buy order
        }
        ImGui::PopStyleColor(3);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.4f, 0.4f, 1.0f));
        if (ImGui::Button("Sell", ImVec2(-1, 45))) {
            // Handle sell order
        }
        ImGui::PopStyleColor(3);
    }
    ImGui::PopFont();

    ImGui::PopStyleVar();

    ImGui::Spacing();

    // Advanced section toggle
    ImGui::PushFont(m_mediumFont);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.4f, 1.0f));
    bool advancedOpen = ImGui::CollapsingHeader("Advanced Sell", ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::PopStyleColor();
    ImGui::PopFont();

    if (advancedOpen) {
        // Stats section
        ImGui::PushFont(m_mediumFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.4f, 1.0f));
        if (ImGui::BeginTabBar("StatsTabBar", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Stats")) {
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Checks")) {
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("My Position")) {
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::PopStyleColor();
        ImGui::PopFont();

        // Time interval buttons
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 8.0f));
        float intervalWidth = (ImGui::GetContentRegionAvail().x - 12.0f) / 4.0f;

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));

        if (ImGui::Button("5M\n-20%", ImVec2(intervalWidth, 45))) {}

        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.4f, 1.0f));

        ImGui::SameLine();
        if (ImGui::Button("1H\n+61%", ImVec2(intervalWidth, 45))) {}

        ImGui::SameLine();
        if (ImGui::Button("6H\n+61%", ImVec2(intervalWidth, 45))) {}

        ImGui::SameLine();
        if (ImGui::Button("24H\n+61%", ImVec2(intervalWidth, 45))) {}

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);

        ImGui::Spacing();
        ImGui::Spacing();

        // Trading metrics section
        if (ImGui::BeginTable("MetricsTable", 3, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Txns");
            ImGui::Text("809");

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Buys");
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), "542");

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Sells");
            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "267");

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Volume");
            ImGui::Text("$122.45K");

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Buy Vol");
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), "$60.82K");

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Sell Vol");
            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "$41.63K");

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Makers");
            ImGui::Text("557");

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Buyers");
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), "476");

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Sellers");
            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "232");

            ImGui::EndTable();
        }
    }

    ImGui::PopStyleVar(4); // Pop all remaining style variables
    ImGui::PopStyleColor(4); // Pop all remaining style colors
    ImGui::End();
}

void TradingUI::UpdateAmountFromPercentage(float percentage) {
    float maxAmount = m_tradingState.buySelected ?
        m_tradingState.availableQuote / m_animationState.displayedPrice :
        m_tradingState.availableBase;

    m_tradingState.amount = maxAmount * percentage;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4) << m_tradingState.amount;
    strncpy(m_tradingState.amountBuf, ss.str().c_str(), IM_ARRAYSIZE(m_tradingState.amountBuf));
}