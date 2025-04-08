#pragma once

#include <string>

// Configuration namespace for application settings
namespace Config {
    // API Settings
    namespace API {
        // Your CoinMarketCap API key (replace with your actual key in production)
        extern std::string CMC_API_KEY;

        // Base URLs for different API endpoints
        extern const std::string CMC_BASE_URL;

        // API request timeouts (in seconds)
        extern const int REQUEST_TIMEOUT;

        // API update intervals (in seconds)
        extern const float PRICE_UPDATE_INTERVAL;
        extern const float CHART_UPDATE_INTERVAL;
    }

    // UI Settings
    namespace UI {
        // Default theme
        extern const bool DEFAULT_DARK_THEME;

        // Default cryptocurrency
        extern const std::string DEFAULT_CRYPTO;

        // Available cryptocurrencies
        extern const std::string AVAILABLE_CRYPTOS[];
        extern const int AVAILABLE_CRYPTOS_COUNT;
    }

    // Application settings
    namespace App {
        extern const wchar_t* APP_TITLE;
        extern const wchar_t* WINDOW_CLASS_NAME;
    }

    void LoadConfig();

}