#include "ChartPanel.h"
#include "CryptoAPIClient.h"
#include "Config.h"
#include "imgui.h"
#include "implot.h"
#include <algorithm>
#include <sstream>
#include <Windows.h>
#include <iostream>

ChartPanel::ChartPanel() {
    // Initialize available symbols from Config
    for (int i = 0; i < Config::UI::AVAILABLE_CRYPTOS_COUNT; i++) {
        m_availableSymbols.push_back(Config::UI::AVAILABLE_CRYPTOS[i]);
    }

    m_symbol = Config::UI::DEFAULT_CRYPTO;
}

ChartPanel::~ChartPanel() {
    // Nothing to clean up
}

void ChartPanel::Initialize(ImFont* boldFont) {
    m_boldFont = boldFont;

    // Initialize chart renderer
    m_chartRenderer.Initialize();
    m_chartRenderer.SetSymbol(m_symbol);
}

void ChartPanel::Render() {
    // Chart title and symbol selector
    ImGui::PushFont(m_boldFont);

    // Symbol selector
    RenderSymbolSelector();

    ImGui::SameLine();
    ImGui::Text("Price Chart");

    // Status indicators and price display
    ImGui::SameLine(ImGui::GetWindowWidth() - 250);

    // API status indicator
    if (m_usingRealData) {
        ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.0f, 1.0f), "LIVE");
    }
    else {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.0f, 1.0f), "MOCK");
    }

    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), "$%.2f", m_displayedPrice);

    ImGui::PopFont();

    ImGui::Spacing();

    // Animate price if needed
    AnimatePrice();

    // Render chart
    m_chartRenderer.RenderCharts();
}

void ChartPanel::RenderSymbolSelector() {
    // Only render if we have symbols
    if (m_availableSymbols.empty()) {
        return;
    }

    // Create a combo box for selecting cryptocurrency
    ImGui::SetNextItemWidth(120);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));

    if (ImGui::BeginCombo("##CryptoSelector", m_symbol.c_str())) {
        for (const auto& symbol : m_availableSymbols) {
            bool isSelected = (symbol == m_symbol);
            if (ImGui::Selectable(symbol.c_str(), isSelected)) {
                if (symbol != m_symbol) {
                    SetSymbol(symbol);

                    // Update the chart data for the new symbol
                    UpdateChartData(symbol);
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
    if (m_isLoading) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Loading...");
    }

    // Show error message if any
    if (!m_errorMessage.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", m_errorMessage.c_str());

        // Clear error after 3 seconds
        float currentTime = ImGui::GetTime();
        static float errorTime = currentTime;
        if (currentTime - errorTime > 3.0f) {
            m_errorMessage.clear();
        }
    }
}

void ChartPanel::AnimatePrice() {
    // Gradually animate towards target price
    if (m_displayedPrice != m_targetPrice) {
        float currentTime = ImGui::GetTime();
        float deltaTime = currentTime - m_lastAnimationUpdate;
        m_lastAnimationUpdate = currentTime;

        float animationDuration = 0.5f; // Animation duration in seconds
        float t = std::min((currentTime - m_priceChangeTime) / animationDuration, 1.0f);
        m_displayedPrice = m_displayedPrice + (m_targetPrice - m_displayedPrice) * t;

        // If we're close enough, snap to the target price
        if (std::abs(m_displayedPrice - m_targetPrice) < 0.01f) {
            m_displayedPrice = m_targetPrice;
        }
    }
}

void ChartPanel::SetAPIClient(std::shared_ptr<CryptoAPIClient> apiClient) {
    m_apiClient = apiClient;
}

void ChartPanel::UpdateChartData(const std::string& symbol) {
    if (!m_apiClient) {
        m_errorMessage = "API client not initialized";
        return;
    }

    // Set loading state
    m_isLoading = true;
    m_errorMessage.clear();

    // Update chart symbol
    m_chartRenderer.SetSymbol(symbol);

    // Fetch historical data for the chart
    m_apiClient->FetchHistoricalData(symbol, [this](const std::vector<PriceData>& historicalData) {
        // Skip if we didn't get any data
        if (historicalData.empty()) {
            m_errorMessage = "No historical data received";
            m_isLoading = false;
            return;
        }

        // Prepare data vectors
        std::vector<double> timestamps;
        std::vector<double> opens;
        std::vector<double> highs;
        std::vector<double> lows;
        std::vector<double> closes;
        std::vector<double> volumes;

        // Reserve space
        timestamps.reserve(historicalData.size());
        opens.reserve(historicalData.size());
        highs.reserve(historicalData.size());
        lows.reserve(historicalData.size());
        closes.reserve(historicalData.size());
        volumes.reserve(historicalData.size());

        // Debug output
        std::stringstream ss;
        ss << "Processing " << historicalData.size() << " historical data points" << std::endl;
        OutputDebugStringA(ss.str().c_str());

        // Fill data vectors
        for (const auto& data : historicalData) {
            timestamps.push_back(data.timestamp);
            opens.push_back(data.open);
            highs.push_back(data.high);
            lows.push_back(data.low);
            closes.push_back(data.close);
            volumes.push_back(data.volume);

            // Debug last data point
            if (&data == &historicalData.back()) {
                std::stringstream debug;
                debug << "Last data point - Date: " << data.timestamp
                    << " Close: " << data.close << std::endl;
                OutputDebugStringA(debug.str().c_str());
            }
        }

        // Update chart renderer with data
        m_chartRenderer.SetChartData(timestamps, opens, highs, lows, closes, volumes);
        m_isLoading = false;
        });

    // Also fetch current price data for display
    m_apiClient->FetchLatestQuote(symbol, [this](const PriceData& priceData, bool isRealData) {
        m_targetPrice = priceData.price;
        m_priceChangeTime = ImGui::GetTime();
        m_usingRealData = isRealData;
        });
}

void ChartPanel::SetSymbol(const std::string& symbol) {
    if (m_symbol != symbol) {
        m_symbol = symbol;
        m_chartRenderer.SetSymbol(symbol);
    }
}