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

    // Set the chart data (new method for API integration)
    void SetChartData(const std::vector<double>& timestamps,
        const std::vector<double>& opens,
        const std::vector<double>& highs,
        const std::vector<double>& lows,
        const std::vector<double>& closes,
        const std::vector<double>& volumes);

    // Set the cryptocurrency symbol for the chart title
    void SetSymbol(const std::string& symbol) { m_symbol = symbol; }

    // Get the current symbol
    const std::string& GetSymbol() const { return m_symbol; }

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

    // Current cryptocurrency symbol
    std::string m_symbol = "ETH";

    // Sample data
    std::vector<double> m_sampleTimestamps;
    std::vector<double> m_sampleOpens;
    std::vector<double> m_sampleHighs;
    std::vector<double> m_sampleLows;
    std::vector<double> m_sampleCloses;
    std::vector<double> m_sampleVolumes;
};