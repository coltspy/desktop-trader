// include/TradingUI.h (revised)
#pragma once

#include "imgui.h"
#include "ChartPanel.h"
#include "PositionsPanel.h"
#include "TradingPanel.h"
#include <memory>
#include "imgui_internal.h" 

class CryptoAPIClient;

class TradingUI {
public:
    TradingUI();
    ~TradingUI() = default;

    void Initialize();
    void Render();
    void SetAPIClient(std::shared_ptr<CryptoAPIClient> apiClient);
    void UpdatePriceData();

private:
    // UI setup
    void SetupStyle();
    void CreateDockingLayout();
    void LoadFonts();
    void RenderMenuBar();

    // Execute trade logic
    void ExecuteTrade(bool isBuy, const std::string& symbol, double price, double amount);

    // UI Components
    ChartPanel m_chartPanel;
    PositionsPanel m_positionsPanel;
    TradingPanel m_tradingPanel;

    // Menu state
    struct {
        bool showDemo = false;
        bool darkTheme = true;
        float userBalance = 25420.36f;
    } m_menuState;

    // API client reference
    std::shared_ptr<CryptoAPIClient> m_apiClient;

    // Font pointers
    ImFont* m_defaultFont = nullptr;
    ImFont* m_boldFont = nullptr;
    ImFont* m_mediumFont = nullptr;

    // DockSpace ID and state
    ImGuiID m_dockspaceId = 0;
    bool m_firstFrame = true;
};