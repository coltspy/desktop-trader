#pragma once

#include "imgui.h"
#include "implot.h"
#include <vector>
#include <string>

// Define chart display modes
enum class ChartDisplayMode {
    Candlestick,
    Line
};

class ChartRenderer {
public:
    ChartRenderer();
    ~ChartRenderer();

    // Initialize ImPlot
    void Initialize();
    // Clean up ImPlot
    void Shutdown();

    // Main rendering function
    void RenderCharts();

    // Set chart display mode
    void SetDisplayMode(ChartDisplayMode mode) { m_displayMode = mode; }
    // Get current display mode
    ChartDisplayMode GetDisplayMode() const { return m_displayMode; }
    // Toggle between display modes
    void ToggleDisplayMode() {
        m_displayMode = (m_displayMode == ChartDisplayMode::Candlestick) ?
            ChartDisplayMode::Line : ChartDisplayMode::Candlestick;
    }

private:
    // Chart rendering methods for different types
    void RenderCandlestickChart();
    void RenderLineChart();

    // Helper function to draw a single candlestick
    void DrawCandlestick(double x, double open, double close, double low, double high,
        const ImVec4& bullColor, const ImVec4& bearColor, double width);

    // Data handling
    void UpdateData();

    // Sample/test data generation
    void GenerateSampleData();

    // Styling for financial charts
    void SetupFinancialChartStyle();

    // Display mode
    ChartDisplayMode m_displayMode = ChartDisplayMode::Candlestick;

    // Sample data
    std::vector<double> m_sampleTimestamps;
    std::vector<double> m_sampleOpens;
    std::vector<double> m_sampleHighs;
    std::vector<double> m_sampleLows;
    std::vector<double> m_sampleCloses;
    std::vector<double> m_sampleVolumes;
};