#pragma once

#include "imgui.h"
#include "ChartRenderer.h"
#include <memory>
#include <string>
#include <vector>

class CryptoAPIClient;
struct PriceData;

class ChartPanel {
public:
    ChartPanel();
    ~ChartPanel();

    void Initialize(ImFont* boldFont);
    void Render();

    // API integration
    void SetAPIClient(std::shared_ptr<CryptoAPIClient> apiClient);
    void UpdateChartData(const std::string& symbol);

    // Getters/Setters
    void SetSymbol(const std::string& symbol);
    const std::string& GetSymbol() const { return m_symbol; }
    float GetCurrentPrice() const { return m_displayedPrice; }

private:
    // UI elements
    void RenderSymbolSelector();
    void AnimatePrice();

    // Chart state
    ChartRenderer m_chartRenderer;
    std::string m_symbol = "ETH";
    bool m_isLoading = false;
    std::string m_errorMessage;
    bool m_usingRealData = false;

    // Price animation state
    float m_displayedPrice = 2500.0f;
    float m_targetPrice = 2500.0f;
    float m_priceChangeTime = 0.0f;
    float m_lastAnimationUpdate = 0.0f;

    // Available symbols
    std::vector<std::string> m_availableSymbols;

    // API client
    std::shared_ptr<CryptoAPIClient> m_apiClient;

    // Font reference
    ImFont* m_boldFont = nullptr;
};