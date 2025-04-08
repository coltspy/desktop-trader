#include "TradingUI.h"
#include "implot.h"
#include "CryptoAPIClient.h"
#include "Config.h"
#include <string>
#include <iomanip>
#include <sstream>

// Font pointers declared in App.cpp
extern ImFont* g_defaultFont;
extern ImFont* g_boldFont;
extern ImFont* g_mediumFont;

TradingUI::TradingUI() {
    // Component initialization happens in Initialize()
}

void TradingUI::Initialize() {
    // Setup style
    SetupStyle();

    // Load fonts from App
    m_defaultFont = g_defaultFont;
    m_boldFont = g_boldFont;
    m_mediumFont = g_mediumFont;

    // Initialize components
    m_chartPanel.Initialize(m_boldFont);
    m_positionsPanel.Initialize(m_boldFont, m_defaultFont);
    m_tradingPanel.Initialize(m_boldFont, m_mediumFont);

    // Set up trading callback
    m_tradingPanel.SetTradeCallback([this](bool isBuy, const std::string& symbol,
        double price, double amount) {
            ExecuteTrade(isBuy, symbol, price, amount);
        });
}

void TradingUI::SetupStyle() {
    // Modern dark theme with flatter, cleaner look
    auto& style = ImGui::GetStyle();

    // Color scheme - cleaner blue accent
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.21f, 1.00f);

    // Primary accent color - blue
    colors[ImGuiCol_Button] = ImVec4(0.16f, 0.29f, 0.48f, 0.60f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.39f, 0.58f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.49f, 0.78f, 1.00f);

    // Headers and tabs
    colors[ImGuiCol_Header] = ImVec4(0.16f, 0.29f, 0.48f, 0.35f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.39f, 0.58f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.39f, 0.58f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.13f, 0.13f, 0.15f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.39f, 0.58f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.33f, 0.53f, 1.00f);

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Buy/Sell colors
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.15f, 0.70f, 0.30f, 1.00f); // Buy/green
    colors[ImGuiCol_PlotLines] = ImVec4(0.75f, 0.20f, 0.20f, 1.00f);     // Sell/red

    // Spacing and rounding
    style.FrameRounding = 4.0f;
    style.FramePadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(10.0f, 6.0f);
    style.WindowRounding = 6.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    // Better scrollbar
    style.ScrollbarSize = 14.0f;
}

void TradingUI::Render() {
    // Render menu bar
    RenderMenuBar();

    // Create fullscreen docking window
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    // Get viewport for full screen
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Clean window border
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGui::Begin("DockSpaceWindow", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // Create dockspace
    m_dockspaceId = ImGui::GetID("MainDockSpace");
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode |
        ImGuiDockNodeFlags_NoDockingInCentralNode |
        ImGuiDockNodeFlags_AutoHideTabBar;

    ImGui::DockSpace(m_dockspaceId, ImVec2(0.0f, 0.0f), dockspace_flags);

    // Create docking layout on first frame
    if (m_firstFrame) {
        m_firstFrame = false;
        CreateDockingLayout();
    }

    // Render all panels
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));

    // Chart Window
    ImGui::Begin("Chart", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    m_chartPanel.Render();
    ImGui::End();

    // Positions Window
    ImGui::Begin("Positions", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    m_positionsPanel.Render();
    ImGui::End();

    // Trading Window
    ImGui::Begin("Trading", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    m_tradingPanel.Render(m_chartPanel.GetSymbol(), m_chartPanel.GetCurrentPrice(), m_menuState.userBalance);
    ImGui::End();

    ImGui::PopStyleVar();

    // Show demos if enabled
    if (m_menuState.showDemo) {
        ImGui::ShowDemoWindow(&m_menuState.showDemo);
    }

    ImGui::End(); // End dockspace window
}

void TradingUI::CreateDockingLayout() {
    // Reset dockspace
    ImGui::DockBuilderRemoveNode(m_dockspaceId);
    ImGui::DockBuilderAddNode(m_dockspaceId, ImGuiDockNodeFlags_DockSpace);

    // Get viewport size
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockBuilderSetNodeSize(m_dockspaceId, viewport->WorkSize);

    // Create layout with three panels
    ImGuiID dock_main_id = m_dockspaceId;

    // Split right side for trading (30% width)
    ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.3f, nullptr, &dock_main_id);

    // Split remaining area for chart (70%) and positions (30%)
    ImGuiID dock_chart_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.7f, nullptr, &dock_main_id);
    ImGuiID dock_positions_id = dock_main_id;

    // Apply lock flags to all nodes
    uint32_t lock_flags = ImGuiDockNodeFlags_NoResize |
        ImGuiDockNodeFlags_NoTabBar |
        ImGuiDockNodeFlags_NoDockingOverMe |
        ImGuiDockNodeFlags_NoDockingInCentralNode;

    ImGuiDockNode* nodes[] = {
        ImGui::DockBuilderGetNode(dock_chart_id),
        ImGui::DockBuilderGetNode(dock_positions_id),
        ImGui::DockBuilderGetNode(dock_right_id)
    };

    for (auto node : nodes) {
        if (node) {
            node->LocalFlags |= lock_flags;
            node->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar;
        }
    }

    // Dock windows
    ImGui::DockBuilderDockWindow("Chart", dock_chart_id);
    ImGui::DockBuilderDockWindow("Positions", dock_positions_id);
    ImGui::DockBuilderDockWindow("Trading", dock_right_id);

    ImGui::DockBuilderFinish(m_dockspaceId);
}

void TradingUI::RenderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Trading")) {
            if (ImGui::MenuItem("Trade History")) {}
            if (ImGui::MenuItem("Account Settings")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Connect Exchange API")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Calculator")) {}
            if (ImGui::MenuItem("Screener")) {}
            if (ImGui::BeginMenu("Indicators")) {
                if (ImGui::MenuItem("Moving Average")) {}
                if (ImGui::MenuItem("RSI")) {}
                if (ImGui::MenuItem("MACD")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("Dark Theme", nullptr, &m_menuState.darkTheme)) {
                // Apply theme updates if needed
                SetupStyle();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Demo")) {
            ImGui::MenuItem("ImGui Demo", nullptr, &m_menuState.showDemo);
            ImGui::EndMenu();
        }

        // Right-aligned balance
        float windowWidth = ImGui::GetWindowWidth();
        float balanceWidth = ImGui::CalcTextSize("Balance: $00,000.00").x;
        ImGui::SameLine(windowWidth - balanceWidth - 20);
        ImGui::Text("Balance: $%.2f", m_menuState.userBalance);

        ImGui::EndMainMenuBar();
    }
}

void TradingUI::SetAPIClient(std::shared_ptr<CryptoAPIClient> apiClient) {
    m_apiClient = apiClient;
    m_chartPanel.SetAPIClient(apiClient);
}

void TradingUI::UpdatePriceData() {
    if (!m_apiClient) {
        return;
    }

    // Get current symbol
    std::string symbol = m_chartPanel.GetSymbol();

    // Update chart data
    m_chartPanel.UpdateChartData(symbol);

    // Update positions with new prices
    m_apiClient->FetchLatestQuote(symbol, [this, symbol](const PriceData& data, bool isRealData) {
        // Update positions with new price data
        m_positionsPanel.UpdatePositionPrice(symbol, data.price);

        // Check for balance changes from positions
        double pnl = m_positionsPanel.GetTotalProfitLoss();

        // Update menu balance (just for display - actual updates happen on trade execution/closing)
        // This is just to reflect unrealized P&L if needed
        });
}

void TradingUI::ExecuteTrade(bool isBuy, const std::string& symbol, double price, double amount) {
    // Validate trade
    double totalCost = price * amount;

    if (isBuy && totalCost > m_menuState.userBalance) {
        // Not enough balance
        return;
    }

    // Create position
    Position newPosition;
    newPosition.symbol = symbol;
    newPosition.type = isBuy ? "Long" : "Short";
    newPosition.entryPrice = price;
    newPosition.amount = amount;
    newPosition.currentPrice = price;
    newPosition.profitLoss = 0.0;
    newPosition.profitLossPercent = 0.0;
    newPosition.isOpen = true;

    // Get current time
    auto now = std::time(nullptr);
    char timeBuffer[30];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    newPosition.openTime = timeBuffer;

    // Update balance
    if (isBuy) {
        m_menuState.userBalance -= totalCost;
    }
    else {
        // For simplicity, same logic for shorts
        m_menuState.userBalance -= totalCost;
    }

    // Add to positions panel
    m_positionsPanel.AddPosition(newPosition);
}