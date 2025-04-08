#include "Config.h"
#include <Windows.h>  // For OutputDebugStringA
#include <fstream>
#include <nlohmann/json.hpp>

namespace Config {
    // API Settings
    namespace API {
        // Make this a variable (not const) so it can be loaded from config
        std::string CMC_API_KEY = "9d71734b-d6d6-44e0-8487-fbabf2c392d6";

        // These remain constants
        const std::string CMC_BASE_URL = "https://pro-api.coinmarketcap.com";
        const int REQUEST_TIMEOUT = 10;
        const float PRICE_UPDATE_INTERVAL = 15.0f;
        const float CHART_UPDATE_INTERVAL = 60.0f;
    }

    // UI Settings - Make sure these are all defined
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

    // Function to load configuration from file
    void LoadConfig() {
        try {
            std::ifstream configFile("config.json");
            if (configFile.is_open()) {
                nlohmann::json config;
                configFile >> config;

                // Load API key if present
                if (config.contains("api") && config["api"].contains("coinmarketcap_key")) {
                    API::CMC_API_KEY = config["api"]["coinmarketcap_key"];
                }

                configFile.close();
            }
            else {
                // Log warning about missing config
                OutputDebugStringA("Warning: config.json not found, using defaults\n");
            }
        }
        catch (const std::exception& e) {
            // Log error
            std::string error = "Error loading config: " + std::string(e.what()) + "\n";
            OutputDebugStringA(error.c_str());
        }
    }
}