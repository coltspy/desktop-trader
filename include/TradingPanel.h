#pragma once

#include "imgui.h"
#include <string>
#include <functional>

// Callback for when a trade is executed
using TradeCallback = std::function<void(bool isBuy, const std::string& symbol,
    double price, double amount)>;

class TradingPanel {
public:
    TradingPanel();
    ~TradingPanel();

    void Initialize(ImFont* boldFont, ImFont* mediumFont);
    void Render(const std::string& symbol, double currentPrice, double balance);

    // Set callback for when a trade is executed
    void SetTradeCallback(TradeCallback callback) { m_tradeCallback = callback; }

private:
    // Update amount based on percentage of available funds
    void UpdateAmountFromPercentage(float percentage, double price, double balance);

    // Trading state
    struct {
        bool buySelected = true;
        float amount = 0.5f;
        float amountPercent = 50.0f;
        char amountBuf[64] = "0.5";
    } m_state;

    // UI references
    ImFont* m_boldFont = nullptr;
    ImFont* m_mediumFont = nullptr;

    // Callback for trade execution
    TradeCallback m_tradeCallback;
};