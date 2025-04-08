#include "TradingPanel.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

TradingPanel::TradingPanel() {
    // Nothing to initialize
}

TradingPanel::~TradingPanel() {
    // Nothing to clean up
}

void TradingPanel::Initialize(ImFont* boldFont, ImFont* mediumFont) {
    m_boldFont = boldFont;
    m_mediumFont = mediumFont;
}

void TradingPanel::Render(const std::string& symbol, double currentPrice, double balance) {
    // Custom styling for trading panel
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));

    // Trading panel title
    ImGui::PushFont(m_boldFont);
    ImGui::Text("Trade %s/USD", symbol.c_str());
    ImGui::PopFont();

    ImGui::Spacing();

    // Amount input
    ImGui::Text("Amount (%s)", symbol.c_str());
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 1.00f));
    if (ImGui::InputText("##AmountInput", m_state.amountBuf, IM_ARRAYSIZE(m_state.amountBuf))) {
        // Parse amount from input field
        try {
            m_state.amount = std::stof(m_state.amountBuf);
        }
        catch (...) {
            // Invalid input - leave amount unchanged
        }
    }
    ImGui::PopStyleColor();

    // Percentage buttons
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));

    float buttonWidth = (ImGui::GetContentRegionAvail().x - 12.0f) / 4.0f;

    ImGui::PushFont(m_mediumFont);

    if (ImGui::Button("25%", ImVec2(buttonWidth, 0))) {
        m_state.amountPercent = 25.0f;
        UpdateAmountFromPercentage(0.25f, currentPrice, balance);
    }

    ImGui::SameLine();
    if (ImGui::Button("50%", ImVec2(buttonWidth, 0))) {
        m_state.amountPercent = 50.0f;
        UpdateAmountFromPercentage(0.5f, currentPrice, balance);
    }

    ImGui::SameLine();
    if (ImGui::Button("75%", ImVec2(buttonWidth, 0))) {
        m_state.amountPercent = 75.0f;
        UpdateAmountFromPercentage(0.75f, currentPrice, balance);
    }

    ImGui::SameLine();
    if (ImGui::Button("100%", ImVec2(buttonWidth, 0))) {
        m_state.amountPercent = 100.0f;
        UpdateAmountFromPercentage(1.0f, currentPrice, balance);
    }

    ImGui::PopFont();
    ImGui::PopStyleVar(); // ItemSpacing

    ImGui::Spacing();

    // Current price display
    ImGui::Text("Current Price: $%.2f", currentPrice);

    ImGui::Spacing();

    // Total cost calculation
    float amount = std::stof(m_state.amountBuf);
    float totalCost = amount * currentPrice;
    ImGui::Text("Total cost: $%.2f", totalCost);

    ImGui::Spacing();

    // Balance display
    ImGui::Text("Available balance: $%.2f", balance);

    ImGui::Spacing();

    // Buy/Sell buttons
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 12.0f));
    ImGui::PushFont(m_boldFont);

    // Buy button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));

    if (ImGui::Button("BUY", ImVec2((ImGui::GetContentRegionAvail().x - 10.0f) / 2, 45))) {
        // Execute buy if we have sufficient balance
        if (amount > 0 && balance >= totalCost) {
            if (m_tradeCallback) {
                m_tradeCallback(true, symbol, currentPrice, amount);
            }
        }
    }

    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    // Sell button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.4f, 0.4f, 1.0f));

    if (ImGui::Button("SELL", ImVec2(ImGui::GetContentRegionAvail().x, 45))) {
        // Execute sell if amount is valid
        if (amount > 0) {
            if (m_tradeCallback) {
                m_tradeCallback(false, symbol, currentPrice, amount);
            }
        }
    }

    ImGui::PopStyleColor(3);

    ImGui::PopFont();
    ImGui::PopStyleVar(); // FramePadding

    // Pop remaining style variables
    ImGui::PopStyleVar(3);
}

void TradingPanel::UpdateAmountFromPercentage(float percentage, double price, double balance) {
    // Calculate maximum amount based on balance
    float maxAmount = balance / price;

    // Calculate amount based on percentage
    m_state.amount = maxAmount * percentage;

    // Format to string with 4 decimal places
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4) << m_state.amount;
    strncpy(m_state.amountBuf, ss.str().c_str(), IM_ARRAYSIZE(m_state.amountBuf));
}