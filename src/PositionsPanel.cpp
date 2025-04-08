#include "PositionsPanel.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

PositionsPanel::PositionsPanel() {
    // Nothing to initialize
}

PositionsPanel::~PositionsPanel() {
    // Nothing to clean up
}

void PositionsPanel::Initialize(ImFont* boldFont, ImFont* regularFont) {
    m_boldFont = boldFont;
    m_regularFont = regularFont;
}

void PositionsPanel::Render() {
    // Custom styling for positions window
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 6.0f));

    // Window title
    ImGui::PushFont(m_boldFont);
    ImGui::Text("Open Positions");
    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Calculate if any positions are open
    bool hasOpenPositions = std::any_of(m_positions.begin(), m_positions.end(),
        [](const Position& p) { return p.isOpen; });

    // Create table for positions
    if (ImGui::BeginTable("PositionsTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Setup headers
        ImGui::TableSetupColumn("Symbol");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Entry Price");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableSetupColumn("Current Price");
        ImGui::TableSetupColumn("P/L");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        // Render open positions
        for (size_t i = 0; i < m_positions.size(); i++) {
            if (!m_positions[i].isOpen) continue;

            const auto& position = m_positions[i];
            ImGui::TableNextRow();

            // Symbol column
            ImGui::TableNextColumn();
            ImGui::Text("%s/USD", position.symbol.c_str());

            // Position type
            ImGui::TableNextColumn();
            if (position.type == "Long") {
                ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), "Long");
            }
            else {
                ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Short");
            }

            // Entry price
            ImGui::TableNextColumn();
            ImGui::Text("$%.2f", position.entryPrice);

            // Amount
            ImGui::TableNextColumn();
            ImGui::Text("%.4f %s", position.amount, position.symbol.c_str());

            // Current price
            ImGui::TableNextColumn();
            ImGui::Text("$%.2f", position.currentPrice);

            // Profit/Loss
            ImGui::TableNextColumn();
            if (position.profitLoss >= 0) {
                ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f),
                    "+$%.2f (%.1f%%)",
                    position.profitLoss,
                    position.profitLossPercent);
            }
            else {
                ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f),
                    "-$%.2f (%.1f%%)",
                    -position.profitLoss,
                    -position.profitLossPercent);
            }

            // Actions column
            ImGui::TableNextColumn();
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::Button("Close")) {
                ClosePosition(i);
            }
            ImGui::PopID();
        }

        // Show empty state message if no positions
        if (!hasOpenPositions) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "No open positions. Use the trading panel to open a position.");

            // Fill remaining columns to complete the row
            for (int i = 0; i < 6; i++) {
                ImGui::TableNextColumn();
            }
        }

        ImGui::EndTable();
    }

    ImGui::PopStyleVar();
}

void PositionsPanel::AddPosition(const Position& position) {
    m_positions.push_back(position);
}

void PositionsPanel::UpdatePositionPrice(const std::string& symbol, double price) {
    for (auto& position : m_positions) {
        if (position.symbol == symbol && position.isOpen) {
            position.currentPrice = price;

            // Calculate profit/loss
            double priceChange = price - position.entryPrice;

            // Calculate P&L based on position type
            if (position.type == "Long") {
                position.profitLoss = priceChange * position.amount;
            }
            else {
                position.profitLoss = -priceChange * position.amount;
            }

            // Calculate percentage change
            position.profitLossPercent = (priceChange / position.entryPrice) * 100.0;
        }
    }
}

void PositionsPanel::ClosePosition(size_t index) {
    if (index < m_positions.size() && m_positions[index].isOpen) {
        // Mark as closed
        m_positions[index].isOpen = false;

        // Call the callback if set
        if (m_positionCloseCallback) {
            m_positionCloseCallback(index, m_positions[index]);
        }
    }
}

double PositionsPanel::GetTotalProfitLoss() const {
    double total = 0.0;

    for (const auto& position : m_positions) {
        if (position.isOpen) {
            total += position.profitLoss;
        }
    }

    return total;
}