#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include <string>

class TradingUI {
public:
    TradingUI();
    ~TradingUI() = default;

    void Initialize();
    void Render();

private:
    // UI setup
    void SetupImGuiStyle();
    void CreateDockingLayout();

    // UI components
    void RenderMenuBar();
    void RenderChartWindow();
    void RenderPositionsWindow();
    void RenderTradingWindow();

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
        float displayedPrice = 1850.45f;
        float targetPrice = 1850.45f;
        float priceChangeTime = 0.0f;
        float lastUpdateTime = 0.0f;
    } m_animationState;

    // DockSpace ID and state
    ImGuiID m_dockspaceId = 0;
    bool m_firstFrame = true;
};