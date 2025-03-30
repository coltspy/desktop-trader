#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "ChartRenderer.h" // Add ChartRenderer include
#include <string>
#include <memory>
#include <vector>
#include <ctime>

// Forward declaration
class CryptoAPIClient;

// Structure to represent a trading position
struct Position {
    std::string symbol;
    std::string type; // "Long" or "Short"
    double entryPrice;
    double amount;
    double currentPrice;
    double profitLoss;
    double profitLossPercent;
    std::string openTime;
    bool isOpen;
};

class TradingUI {
public:
    TradingUI();
    ~TradingUI() = default;

    void Initialize();
    void Render();

    // Set the API client
    void SetAPIClient(std::shared_ptr<CryptoAPIClient> apiClient) { m_apiClient = apiClient; }

    // Get the selected cryptocurrency symbol
    const std::string& GetSelectedSymbol() const { return m_cryptoState.selectedSymbol; }

    // Update the price data from the API
    void UpdatePriceData();

private:
    // UI setup
    void SetupImGuiStyle();
    void CreateDockingLayout();

    // Font handling
    void LoadFonts();

    // UI components
    void RenderMenuBar();
    void RenderChartWindow();
    void RenderPositionsWindow();
    void RenderTradingWindow();

    // New method for cryptocurrency selection
    void RenderCryptoSelector();

    // Helper function for updating amount from percentage buttons
    void UpdateAmountFromPercentage(float percentage);

    // Helper function to execute trades
    void ExecuteTrade(bool isBuy, const std::string& symbol, double price, double amount);

    // Helper function to update positions with new prices
    void UpdatePositions(const std::string& symbol, double currentPrice);

    // Menu state
    struct {
        bool showDemo = false;
        bool showImPlotDemo = false; // Add ImPlot demo toggle
        bool darkTheme = true;
        bool showOrderBook = true;
        bool showDepthChart = false;
        float userBalance = 25420.36f;
    } m_menuState;

    // Trading panel state
    struct {
        bool buySelected = true;
        int orderType = 0; // 0=Market, 1=Limit
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

    // Cryptocurrency selection state
    struct {
        std::vector<std::string> availableSymbols;
        std::string selectedSymbol;
        bool isLoading = false;
        std::string errorMessage;
        bool usingRealApiData = false; // Flag to indicate if we're using real API data
    } m_cryptoState;

    // Font pointers
    ImFont* m_defaultFont = nullptr;
    ImFont* m_boldFont = nullptr;
    ImFont* m_mediumFont = nullptr;
    ImFont* m_smallFont = nullptr;

    // Chart renderer
    ChartRenderer m_chartRenderer;

    // API client reference
    std::shared_ptr<CryptoAPIClient> m_apiClient;

    // DockSpace ID and state
    ImGuiID m_dockspaceId = 0;
    bool m_firstFrame = true;

    // Vector to store positions
    std::vector<Position> m_positions;
};