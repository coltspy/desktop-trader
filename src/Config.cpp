#include "Config.h"

namespace Config {
    // API Settings
    namespace API {
        // Your CoinMarketCap API key (replace with your actual key in production)
        const std::string CMC_API_KEY = "";

        // Base URLs for different API endpoints
        const std::string CMC_BASE_URL = "https://pro-api.coinmarketcap.com";

        // API request timeouts (in seconds)
        const int REQUEST_TIMEOUT = 10;

        // API update intervals (in seconds)
        const float PRICE_UPDATE_INTERVAL = 15.0f;
        const float CHART_UPDATE_INTERVAL = 60.0f;
    }

    // UI Settings
    namespace UI {
        // Default theme
        const bool DEFAULT_DARK_THEME = true;

        // Default cryptocurrency
        const std::string DEFAULT_CRYPTO = "ETH";

        // Available cryptocurrencies
        const std::string AVAILABLE_CRYPTOS[] = { "BTC", "ETH", "USDT", "SOL", "XRP", "BNB", "ADA", "DOT" };
        const int AVAILABLE_CRYPTOS_COUNT = sizeof(AVAILABLE_CRYPTOS) / sizeof(AVAILABLE_CRYPTOS[0]);
    }

    // Application settings
    namespace App {
        const wchar_t* APP_TITLE = L"Crypto Trading Platform";
        const wchar_t* WINDOW_CLASS_NAME = L"CryptoTradingPlatform";
    }
}