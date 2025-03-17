#include "ChartRenderer.h"
#include <ctime>
#include <random>
#include <algorithm>
#include <cmath>

ChartRenderer::ChartRenderer() {
    // Generate some sample data for initial display
    GenerateSampleData();
}

ChartRenderer::~ChartRenderer() {
}

void ChartRenderer::Initialize() {
    // Initialize ImPlot if not already initialized
    if (ImPlot::GetCurrentContext() == nullptr) {
        ImPlot::CreateContext();
    }

    // Setup ImPlot style
    SetupFinancialChartStyle();
}

void ChartRenderer::Shutdown() {
    // Cleanup ImPlot is handled in App.cpp
}

void ChartRenderer::SetupFinancialChartStyle() {
    ImPlotStyle& style = ImPlot::GetStyle();

    // Set colors appropriate for trading platform
    style.Colors[ImPlotCol_Line] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImPlotCol_Fill] = ImVec4(0.16f, 0.29f, 0.48f, 0.50f);
    style.Colors[ImPlotCol_MarkerFill] = ImVec4(1.00f, 0.44f, 0.00f, 1.00f);
    style.Colors[ImPlotCol_MarkerOutline] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImPlotCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImPlotCol_PlotBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    style.Colors[ImPlotCol_PlotBorder] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImPlotCol_LegendBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.70f);
    style.Colors[ImPlotCol_LegendBorder] = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
    style.Colors[ImPlotCol_LegendText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImPlotCol_TitleText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImPlotCol_InlayText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImPlotCol_AxisText] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    style.Colors[ImPlotCol_AxisGrid] = ImVec4(0.30f, 0.30f, 0.30f, 0.25f);
    style.Colors[ImPlotCol_AxisTick] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImPlotCol_AxisBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImPlotCol_Selection] = ImVec4(1.00f, 0.60f, 0.00f, 0.25f);

    // Set other styling properties for financial charts
    style.LineWeight = 1.5f;
    style.MarkerSize = 3.0f;
    style.PlotDefaultSize = ImVec2(640, 480);
    style.PlotMinSize = ImVec2(300, 225);
}

void ChartRenderer::GenerateSampleData() {
    // Generate sample OHLC data for demonstration
    const int num_points = 100;
    double time_now = (double)time(nullptr);
    double time_step = 24 * 60 * 60; // Daily data

    m_sampleTimestamps.resize(num_points);
    m_sampleOpens.resize(num_points);
    m_sampleHighs.resize(num_points);
    m_sampleLows.resize(num_points);
    m_sampleCloses.resize(num_points);
    m_sampleVolumes.resize(num_points);

    // Random generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(0, 1);

    // Start with a price of 100
    double price = 100.0;

    for (int i = 0; i < num_points; ++i) {
        // Time data (going back in time from now)
        m_sampleTimestamps[i] = time_now - (num_points - i) * time_step;

        // Price data with a random walk
        double change = d(gen) * 2.0; // Random daily change
        double open = price;
        price += change;
        double close = price;

        double high = std::max(open, close) + std::abs(d(gen)) * 0.5;
        double low = std::min(open, close) - std::abs(d(gen)) * 0.5;

        m_sampleOpens[i] = open;
        m_sampleHighs[i] = high;
        m_sampleLows[i] = low;
        m_sampleCloses[i] = close;

        // Volume data (random, higher on big price moves)
        m_sampleVolumes[i] = 1000000 + std::abs(change) * 200000 + d(gen) * 100000;
    }
}

void ChartRenderer::UpdateData() {
    // In a real application, this would fetch new data from a market data provider
    // For now, we just use the sample data
}

void ChartRenderer::RenderCharts() {
    // Update our data first
    UpdateData();

    // Chart type selector as buttons at the top
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5));

    if (ImGui::Button("Candlestick", ImVec2(120, 30))) {
        m_displayMode = ChartDisplayMode::Candlestick;
    }

    ImGui::SameLine();

    if (ImGui::Button("Line", ImVec2(120, 30))) {
        m_displayMode = ChartDisplayMode::Line;
    }

    ImGui::PopStyleVar();
    ImGui::Spacing();

    // Render based on current display mode
    switch (m_displayMode) {
    case ChartDisplayMode::Candlestick:
        RenderCandlestickChart();
        break;
    case ChartDisplayMode::Line:
        RenderLineChart();
        break;
    }
}

void ChartRenderer::DrawCandlestick(double x, double open, double close, double low, double high,
    const ImVec4& bullColor, const ImVec4& bearColor, double width) {
    // Determine if this is a bullish or bearish candle
    bool bullish = close >= open;
    ImU32 color = ImGui::GetColorU32(bullish ? bullColor : bearColor);

    // Get the current plot
    ImPlotContext* ctx = ImPlot::GetCurrentContext();
    ImDrawList* draw_list = ImPlot::GetPlotDrawList();

    // Convert data positions to pixel positions
    ImVec2 lowPoint = ImPlot::PlotToPixels(x, low);
    ImVec2 highPoint = ImPlot::PlotToPixels(x, high);
    ImVec2 openLeft = ImPlot::PlotToPixels(x - width / 2.0, open);
    ImVec2 openRight = ImPlot::PlotToPixels(x + width / 2.0, open);
    ImVec2 closeLeft = ImPlot::PlotToPixels(x - width / 2.0, close);
    ImVec2 closeRight = ImPlot::PlotToPixels(x + width / 2.0, close);

    // Draw the wick (vertical line from low to high)
    draw_list->AddLine(lowPoint, highPoint, color, 1.0f);

    // Draw the body rectangle
    if (bullish) {
        // For bullish candles, body goes from open to close (bottom to top)
        draw_list->AddRectFilled(
            ImVec2(openLeft.x, openLeft.y),
            ImVec2(closeRight.x, closeRight.y),
            color
        );
    }
    else {
        // For bearish candles, body goes from close to open (bottom to top)
        draw_list->AddRectFilled(
            ImVec2(closeLeft.x, closeLeft.y),
            ImVec2(openRight.x, openRight.y),
            color
        );
    }
}

void ChartRenderer::RenderCandlestickChart() {
    // Use the full available space
    ImVec2 availableSize = ImGui::GetContentRegionAvail();

    // Plot candlestick chart
    if (ImPlot::BeginPlot("ETH/USD", availableSize)) {
        // Setup axes
        ImPlot::SetupAxes("Time", "Price", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        ImPlot::SetupAxisLimits(ImAxis_X1, m_sampleTimestamps.front(), m_sampleTimestamps.back());

        // Get min/max price for Y-axis limits
        double min_price = *std::min_element(m_sampleLows.begin(), m_sampleLows.end());
        double max_price = *std::max_element(m_sampleHighs.begin(), m_sampleHighs.end());
        double price_range = max_price - min_price;
        ImPlot::SetupAxisLimits(ImAxis_Y1, min_price - 0.1 * price_range, max_price + 0.1 * price_range);
        ImPlot::SetupAxisFormat(ImAxis_Y1, "$%.2f");

        // Define colors for up/down candles
        ImVec4 bullCol = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // Green for up
        ImVec4 bearCol = ImVec4(0.8f, 0.0f, 0.0f, 1.0f);  // Red for down

        // Calculate width for candlesticks (60% of the interval between points)
        double width = 0.6;
        if (m_sampleTimestamps.size() > 1) {
            width = 0.6 * (m_sampleTimestamps[1] - m_sampleTimestamps[0]);
        }

        // Draw candlesticks manually
        for (size_t i = 0; i < m_sampleTimestamps.size(); ++i) {
            DrawCandlestick(
                m_sampleTimestamps[i],
                m_sampleOpens[i],
                m_sampleCloses[i],
                m_sampleLows[i],
                m_sampleHighs[i],
                bullCol,
                bearCol,
                width
            );
        }

        // Plot a simple moving average
        if (m_sampleTimestamps.size() >= 20) {
            std::vector<double> sma20(m_sampleTimestamps.size());
            for (int i = 0; i < m_sampleTimestamps.size(); ++i) {
                if (i < 19) {
                    sma20[i] = 0; // Not enough data yet
                }
                else {
                    double sum = 0;
                    for (int j = i - 19; j <= i; ++j) {
                        sum += m_sampleCloses[j];
                    }
                    sma20[i] = sum / 20.0;
                }
            }

            ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 0.0f, 0.8f), 2.0f);
            ImPlot::PlotLine("20 SMA", m_sampleTimestamps.data(), sma20.data(), (int)sma20.size());
        }

        ImPlot::EndPlot();
    }
}

void ChartRenderer::RenderLineChart() {
    // Use the full available space
    ImVec2 availableSize = ImGui::GetContentRegionAvail();

    if (ImPlot::BeginPlot("ETH/USD", availableSize)) {
        // Setup axes
        ImPlot::SetupAxes("Time", "Price", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        ImPlot::SetupAxisLimits(ImAxis_X1, m_sampleTimestamps.front(), m_sampleTimestamps.back());
        ImPlot::SetupAxisFormat(ImAxis_Y1, "$%.2f");

        // Get min/max price for Y-axis limits
        double min_price = *std::min_element(m_sampleLows.begin(), m_sampleLows.end());
        double max_price = *std::max_element(m_sampleHighs.begin(), m_sampleHighs.end());
        double price_range = max_price - min_price;
        ImPlot::SetupAxisLimits(ImAxis_Y1, min_price - 0.1 * price_range, max_price + 0.1 * price_range);

        // Plot closing prices as a line series
        ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), 2.0f);
        ImPlot::PlotLine("Price", m_sampleTimestamps.data(), m_sampleCloses.data(), (int)m_sampleCloses.size());

        // Plot a simple moving average
        if (m_sampleTimestamps.size() >= 20) {
            std::vector<double> sma20(m_sampleTimestamps.size());
            for (int i = 0; i < m_sampleTimestamps.size(); ++i) {
                if (i < 19) {
                    sma20[i] = 0; // Not enough data yet
                }
                else {
                    double sum = 0;
                    for (int j = i - 19; j <= i; ++j) {
                        sum += m_sampleCloses[j];
                    }
                    sma20[i] = sum / 20.0;
                }
            }

            ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 0.0f, 0.8f), 2.0f);
            ImPlot::PlotLine("20 SMA", m_sampleTimestamps.data(), sma20.data(), (int)sma20.size());
        }

        ImPlot::EndPlot();
    }
}