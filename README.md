# Crypto Trading Platform

A modern cryptocurrency trading platform interface built with Dear ImGui and DirectX 11, featuring real-time cryptocurrency data from CoinMarketCap API.

## Overview

This project is a desktop trading platform for cryptocurrencies with a professional UI featuring:
- Real-time price updates via CoinMarketCap API
- Interactive trading panels
- Position management
- Price charts with multiple display options
- Multiple cryptocurrency support

## Technologies Used

- C++17
- Dear ImGui (Docking branch)
- ImPlot for financial charting
- DirectX 11 for rendering
- libcurl for API communication
- nlohmann/json for JSON parsing
- CMake for build configuration

## Features

- **Responsive UI** with dockable windows and dynamic layout
- **CoinMarketCap API Integration** for real-time crypto data
- **Trading Panel** with:
  - Buy/Sell tabs
  - Multiple order types (Market, Limit)
  - Percentage-based amount selection
  - Real-time price display with animations
  - Fee calculation
- **Cryptocurrency Selection** supporting multiple major cryptocurrencies
- **Positions Window** for tracking open trades
- **Interactive Chart Window** with:
  - Candlestick and line chart options
  - Moving average indicators
  - Historical price data
- **Dark Theme** with modern styling

## API Configuration

Before running the application, update your CoinMarketCap API key in `include/Config.h`:

```cpp
namespace API {
    // Your CoinMarketCap API key
    const std::string CMC_API_KEY = "YOUR_API_KEY_HERE";
    // ...
}
```

You can get a free API key at [coinmarketcap.com/api](https://coinmarketcap.com/api/).

## Building the Project

### Prerequisites

- Visual Studio 2019 or 2022 with C++ desktop development
- CMake 3.14 or higher
- Git

### Build Steps

1. Clone the repository
2. Configure with CMake:
   ```
   mkdir build
   cd build
   cmake ..
   ```
3. Build the project:
   ```
   cmake --build .
   ```
   
Alternatively, open the project in Visual Studio after CMake configuration.

## Usage

1. Launch the application
2. Select a cryptocurrency from the dropdown at the top of the chart
3. View real-time price data and charts
4. Use the trading panel to simulate trades

## Adding More Cryptocurrencies

To add more cryptocurrencies to the platform, modify the `AVAILABLE_CRYPTOS` array in `include/Config.h`.

## License

This project is licensed under the MIT License - see the LICENSE file for details.