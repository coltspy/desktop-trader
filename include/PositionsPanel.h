#pragma once

#include "imgui.h"
#include <string>
#include <vector>
#include <functional>

// Position struct to represent a trading position
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

// Callback for when a position is closed
using PositionCallback = std::function<void(size_t index, const Position& position)>;

class PositionsPanel {
public:
    PositionsPanel();
    ~PositionsPanel();

    void Initialize(ImFont* boldFont, ImFont* regularFont);
    void Render();

    // Position management
    void AddPosition(const Position& position);
    void UpdatePositionPrice(const std::string& symbol, double price);
    void ClosePosition(size_t index);

    // Set callback for when positions are closed
    void SetPositionCloseCallback(PositionCallback callback) { m_positionCloseCallback = callback; }

    // Calculate total profit/loss from all open positions
    double GetTotalProfitLoss() const;

    // Get position data
    const std::vector<Position>& GetPositions() const { return m_positions; }

private:
    // Positions data
    std::vector<Position> m_positions;

    // Callback for position closing
    PositionCallback m_positionCloseCallback;

    // Font references
    ImFont* m_boldFont = nullptr;
    ImFont* m_regularFont = nullptr;
};