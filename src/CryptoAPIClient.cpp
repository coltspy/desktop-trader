#include "CryptoAPIClient.h"
#include "Config.h"
#include "SimpleHttpClient.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <ctime>
#include <random>
#include <algorithm>
#include <chrono>
#include <unordered_map>

// Implementation using WinHttp for real API calls
CryptoAPIClient::CryptoAPIClient() : m_shouldStop(false) {
}

CryptoAPIClient::~CryptoAPIClient() {
    Shutdown();
}

bool CryptoAPIClient::Initialize(const std::string& apiKey) {
    m_apiKey = apiKey;
    m_baseUrl = Config::API::CMC_BASE_URL;

    // Start the request processing thread
    m_shouldStop = false;
    m_requestThread = std::make_unique<std::thread>(&CryptoAPIClient::ProcessRequests, this);

    return true;
}

void CryptoAPIClient::Shutdown() {
    if (m_requestThread) {
        // Signal the thread to stop
        m_shouldStop = true;
        m_queueCondition.notify_one();

        // Wait for the thread to finish
        if (m_requestThread->joinable()) {
            m_requestThread->join();
        }
        m_requestThread.reset();
    }
}

bool CryptoAPIClient::FetchLatestQuote(const std::string& symbol,
    std::function<void(const PriceData&, bool)> callback) {
    // Skip if no API key configured
    if (m_apiKey.empty()) {
        m_lastError = "API key not configured";
        callback(GenerateMockPriceData(symbol), false);
        return false;
    }

    // Set up endpoint and parameters
    std::string endpoint = "/v1/cryptocurrency/quotes/latest";
    std::map<std::string, std::string> params = {
        {"symbol", symbol},
        {"convert", "USD"}
    };

    // Queue the request with retry and timeout handling
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_requestQueue.push_back({
        endpoint,
        params,
        [this, symbol, callback](const std::string& response) {
            try {
                // Parse JSON response
                auto json = nlohmann::json::parse(response);

                // Error handling - check API errors
                if (json.contains("status") && json["status"].contains("error_code") &&
                    json["status"]["error_code"] != 0) {

                    m_lastError = "API Error: " + json["status"]["error_message"].get<std::string>();
                    callback(GenerateMockPriceData(symbol), false);
                    return;
                }

                // Extract price data
                PriceData data;
                data.symbol = symbol;

                // Extract all the relevant fields, with error checking
                if (json["data"].contains(symbol) &&
                    json["data"][symbol].contains("quote") &&
                    json["data"][symbol]["quote"].contains("USD")) {

                    auto& usdData = json["data"][symbol]["quote"]["USD"];

                    data.price = usdData.contains("price") ? usdData["price"].get<double>() : 0.0;
                    data.volume24h = usdData.contains("volume_24h") ? usdData["volume_24h"].get<double>() : 0.0;
                    data.percentChange1h = usdData.contains("percent_change_1h") ?
                                         usdData["percent_change_1h"].get<double>() : 0.0;
                    data.percentChange24h = usdData.contains("percent_change_24h") ?
                                          usdData["percent_change_24h"].get<double>() : 0.0;
                    data.lastUpdated = json["data"][symbol].contains("last_updated") ?
                                     json["data"][symbol]["last_updated"].get<std::string>() : "";

                    callback(data, true);
                }
 else {
  m_lastError = "API response missing required data fields";
  callback(GenerateMockPriceData(symbol), false);
}
}
catch (const std::exception& e) {
    m_lastError = "Error parsing response: " + std::string(e.what());
    callback(GenerateMockPriceData(symbol), false);
}
}
        });

    // Start processing thread if needed
    if (!m_requestThread || !m_requestThread->joinable()) {
        m_shouldStop = false;
        m_requestThread = std::make_unique<std::thread>(&CryptoAPIClient::ProcessRequests, this);
    }

    m_queueCondition.notify_one();
    return true;
}

// Cache for mock price data to ensure consistency
static std::unordered_map<std::string, PriceData> mockPriceCache;

PriceData CryptoAPIClient::GenerateMockPriceData(const std::string& symbol) {
    // Check if we already have mock data for this symbol
    if (mockPriceCache.count(symbol) > 0) {
        // Update the timestamp
        time_t now = time(nullptr);
        char timeBuffer[30];
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%dT%H:%M:%S.000Z", gmtime(&now));
        mockPriceCache[symbol].lastUpdated = timeBuffer;

        // Add a small random variance to the price (±0.5%) to simulate market movement
        double variance = (rand() % 100 - 50) * 0.0001; // -0.5% to +0.5%
        mockPriceCache[symbol].price *= (1.0 + variance);

        // Update OHLC data
        mockPriceCache[symbol].open = mockPriceCache[symbol].price;
        mockPriceCache[symbol].high = mockPriceCache[symbol].price * 1.005;
        mockPriceCache[symbol].low = mockPriceCache[symbol].price * 0.995;
        mockPriceCache[symbol].close = mockPriceCache[symbol].price;

        return mockPriceCache[symbol];
    }

    // Use a fixed seed based on the symbol to ensure consistency
    std::hash<std::string> hasher;
    size_t hash = hasher(symbol);
    std::srand(static_cast<unsigned int>(hash));

    PriceData mockData;
    mockData.symbol = symbol;

    if (symbol == "BTC") {
        mockData.price = 65000.0 + (std::rand() % 2000 - 1000);
    }
    else if (symbol == "ETH") {
        mockData.price = 2500.0 + (std::rand() % 100 - 50);
    }
    else if (symbol == "USDT") {
        mockData.price = 1.0 + (std::rand() % 2 - 1) * 0.01;
    }
    else if (symbol == "SOL") {
        mockData.price = 150.0 + (std::rand() % 10 - 5);
    }
    else if (symbol == "XRP") {
        mockData.price = 0.5 + (std::rand() % 10 - 5) * 0.01;
    }
    else if (symbol == "BNB") {
        mockData.price = 350.0 + (std::rand() % 20 - 10);
    }
    else if (symbol == "ADA") {
        mockData.price = 0.4 + (std::rand() % 10 - 5) * 0.01;
    }
    else if (symbol == "DOT") {
        mockData.price = 8.0 + (std::rand() % 100 - 50) * 0.01;
    }
    else {
        mockData.price = 100.0 + (std::rand() % 20 - 10);
    }

    mockData.volume24h = (std::rand() % 1000 + 500) * 1000000;
    mockData.percentChange1h = (std::rand() % 400 - 200) * 0.01;
    mockData.percentChange24h = (std::rand() % 1000 - 500) * 0.01;
    mockData.percentChange7d = (std::rand() % 2000 - 1000) * 0.01;
    mockData.marketCap = mockData.price * (std::rand() % 10 + 10) * 1000000;

    time_t now = time(nullptr);
    char timeBuffer[30];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%dT%H:%M:%S.000Z", gmtime(&now));
    mockData.lastUpdated = timeBuffer;

    // Copy to OHLC data
    mockData.open = mockData.price;
    mockData.high = mockData.price * 1.005;
    mockData.low = mockData.price * 0.995;
    mockData.close = mockData.price;

    // Cache the mock data
    mockPriceCache[symbol] = mockData;

    return mockData;
}

// Cache for historical data to ensure consistency
static std::unordered_map<std::string, std::vector<PriceData>> historicalDataCache;

bool CryptoAPIClient::FetchHistoricalData(const std::string& symbol,
    std::function<void(const std::vector<PriceData>&)> callback) {
    if (m_apiKey.empty()) {
        m_lastError = "API key not configured";
        callback(std::vector<PriceData>());
        return false;
    }

    // We'll use the listings/historical endpoint which is available in your API tier
    std::string endpoint = "/v1/cryptocurrency/listings/historical";

    // Create a vector to store historical data points
    std::vector<PriceData> historicalData;

    // We need to fetch multiple days to build chart data
    // Let's get data for the last 30 days
    time_t now = time(nullptr);

    // Process one day at a time to build the chart data
    auto processDayData = [this, &historicalData, symbol, callback, now](int daysAgo) {
        // Calculate date
        time_t dayTime = now - (daysAgo * 24 * 60 * 60);
        char dateStr[11]; // YYYY-MM-DD
        strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", gmtime(&dayTime));

        // Parameters for this specific day
        std::map<std::string, std::string> params = {
            {"date", dateStr},
            {"limit", "5000"},  // High limit to ensure we get all cryptos
            {"convert", "USD"}
        };

        // Make the API request for this day
        std::string response;
        if (!MakeRequest("/v1/cryptocurrency/listings/historical", params, response)) {
            OutputDebugStringA(("Failed to get data for " + std::string(dateStr) + "\n").c_str());
            return false;
        }

        try {
            // Parse the response
            auto json = nlohmann::json::parse(response);

            // Check for API errors
            if (json.contains("status") && json["status"].contains("error_code") &&
                json["status"]["error_code"] != 0) {

                m_lastError = "API Error: " + json["status"]["error_message"].get<std::string>();
                OutputDebugStringA(m_lastError.c_str());
                return false;
            }

            // Find the specific crypto in the data array
            bool found = false;
            if (json.contains("data") && json["data"].is_array()) {
                for (const auto& crypto : json["data"]) {
                    if (crypto.contains("symbol") && crypto["symbol"] == symbol) {
                        // Found our cryptocurrency
                        PriceData data;
                        data.symbol = symbol;
                        data.timestamp = dayTime;

                        // Get price data
                        if (crypto.contains("quote") && crypto["quote"].contains("USD")) {
                            data.close = crypto["quote"]["USD"]["price"].get<double>();
                            data.volume = crypto["quote"]["USD"]["volume_24h"].get<double>();

                            // For OHLC, we only have close price, so approximate others
                            double priceChange = crypto["quote"]["USD"]["percent_change_24h"].get<double>() / 100.0;
                            data.open = data.close / (1.0 + priceChange);

                            // Approximate high/low based on daily volatility
                            double volatility = std::abs(priceChange) * 1.5;
                            data.high = data.close * (1.0 + volatility / 2);
                            data.low = data.close * (1.0 - volatility / 2);

                            historicalData.push_back(data);
                            found = true;
                            break;
                        }
                    }
                }
            }

            if (!found) {
                OutputDebugStringA(("Symbol " + symbol + " not found for " + std::string(dateStr) + "\n").c_str());
            }

            return found;
        }
        catch (const std::exception& e) {
            m_lastError = "Error parsing data for " + std::string(dateStr) + ": " + std::string(e.what());
            OutputDebugStringA(m_lastError.c_str());
            return false;
        }
        };

    // Start with latest data using listings/latest for most accurate current price
    std::map<std::string, std::string> latestParams = {
        {"limit", "5000"},
        {"convert", "USD"}
    };

    std::string latestResponse;
    if (MakeRequest("/v1/cryptocurrency/listings/latest", latestParams, latestResponse)) {
        try {
            auto json = nlohmann::json::parse(latestResponse);

            // Find the current price
            if (json.contains("data") && json["data"].is_array()) {
                for (const auto& crypto : json["data"]) {
                    if (crypto.contains("symbol") && crypto["symbol"] == symbol) {
                        PriceData data;
                        data.symbol = symbol;
                        data.timestamp = now;

                        if (crypto.contains("quote") && crypto["quote"].contains("USD")) {
                            data.close = crypto["quote"]["USD"]["price"].get<double>();
                            data.volume = crypto["quote"]["USD"]["volume_24h"].get<double>();

                            // For OHLC, we only have close price, so approximate others
                            double priceChange = crypto["quote"]["USD"]["percent_change_24h"].get<double>() / 100.0;
                            data.open = data.close / (1.0 + priceChange);

                            // Approximate high/low based on daily volatility
                            double volatility = std::abs(priceChange) * 1.5;
                            data.high = data.close * (1.0 + volatility / 2);
                            data.low = data.close * (1.0 - volatility / 2);

                            historicalData.push_back(data);
                            break;
                        }
                    }
                }
            }
        }
        catch (const std::exception& e) {
            OutputDebugStringA(("Error parsing latest data: " + std::string(e.what()) + "\n").c_str());
        }
    }

    // Now get historical data for previous days
    for (int day = 1; day <= 30; day++) {
        processDayData(day);
    }

    // Sort by timestamp (oldest to newest)
    std::sort(historicalData.begin(), historicalData.end(),
        [](const PriceData& a, const PriceData& b) {
            return a.timestamp < b.timestamp;
        });

    // Log what we found
    OutputDebugStringA(("Retrieved " + std::to_string(historicalData.size()) +
        " data points for " + symbol + "\n").c_str());

    if (!historicalData.empty()) {
        callback(historicalData);
        return true;
    }

    callback(std::vector<PriceData>());
    return false;
}

void CryptoAPIClient::GenerateMockHistoricalData(const std::string& symbol, std::vector<PriceData>& data, int numDays) {
    // Use a fixed seed based on the symbol to ensure consistency
    std::hash<std::string> hasher;
    size_t hash = hasher(symbol);
    std::srand(static_cast<unsigned int>(hash));

    // Current time in seconds since epoch
    double now = static_cast<double>(std::time(nullptr));
    double daySeconds = 24 * 60 * 60;

    // Set base price based on symbol
    double basePrice = 100.0;
    if (symbol == "BTC") {
        basePrice = 65000.0;
    }
    else if (symbol == "ETH") {
        basePrice = 2500.0;
    }
    else if (symbol == "USDT") {
        basePrice = 1.0;
    }
    else if (symbol == "SOL") {
        basePrice = 150.0;
    }
    else if (symbol == "XRP") {
        basePrice = 0.5;
    }
    else if (symbol == "BNB") {
        basePrice = 350.0;
    }
    else if (symbol == "ADA") {
        basePrice = 0.4;
    }
    else if (symbol == "DOT") {
        basePrice = 8.0;
    }

    // Generate data for the past numDays days
    data.resize(numDays);

    // We'll use a simple random walk to generate price movements
    double price = basePrice;

    for (int i = 0; i < numDays; ++i) {
        // Calculate timestamp for this day (going backward from now)
        double timestamp = now - (numDays - i) * daySeconds;

        // Generate a random price change (-2% to +2% of base price)
        double changePercent = (std::rand() % 400 - 200) * 0.0001; // -0.02 to 0.02
        double change = price * changePercent;

        // Calculate OHLC values
        double open = price;
        price += change;
        double close = price;

        // High and low are derived from open and close
        double high = std::max(open, close) * (1.0 + (std::rand() % 50) * 0.0001); // Up to 0.5% higher
        double low = std::min(open, close) * (1.0 - (std::rand() % 50) * 0.0001);  // Up to 0.5% lower

        // Create the data point
        PriceData& point = data[i];
        point.symbol = symbol;
        point.timestamp = timestamp;
        point.open = open;
        point.high = high;
        point.low = low;
        point.close = close;
        point.volume = basePrice * (std::rand() % 1000 + 500) * 10.0; // Random volume

        // Also set the price fields for convenience
        point.price = close;
    }
}

bool CryptoAPIClient::MakeRequest(const std::string& endpoint, const std::map<std::string, std::string>& params, std::string& response) {
    try {
        // Build the URL with query parameters
        std::string url = m_baseUrl + endpoint + "?";
        bool first = true;

        for (const auto& param : params) {
            if (!first) {
                url += "&";
            }
            url += param.first + "=" + param.second;
            first = false;
        }

        // Log the request (without the API key for security)
        std::cout << "Making API request to: " << url << std::endl;

        // Setup headers
        std::map<std::string, std::string> headers = {
            {"X-CMC_PRO_API_KEY", m_apiKey},
            {"Accept", "application/json"}
        };

        // Make the HTTP request
        std::string error;
        bool success = SimpleHttpClient::Get(url, headers, response, error);

        if (!success) {
            m_lastError = "HTTP request failed: " + error;
            std::cerr << m_lastError << std::endl;
            return false;
        }

        // Log successful response (partial, for debugging)
        if (response.length() > 0) {
            std::cout << "Received API response (" << response.length()
                << " bytes): " << response.substr(0, 100) << "..." << std::endl;
        }

        return true;
    }
    catch (const std::exception& e) {
        m_lastError = "Request error: " + std::string(e.what());
        std::cerr << m_lastError << std::endl;
        return false;
    }
}

void CryptoAPIClient::ProcessRequests() {
    while (!m_shouldStop) {
        APIRequest request;

        // Wait for a request
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_queueCondition.wait(lock, [this] { return m_shouldStop || !m_requestQueue.empty(); });

            if (m_shouldStop && m_requestQueue.empty()) {
                break;
            }

            if (!m_requestQueue.empty()) {
                request = m_requestQueue.front();
                m_requestQueue.erase(m_requestQueue.begin());
            }
        }

        // Process the request
        std::string response;
        if (MakeRequest(request.endpoint, request.params, response)) {
            // Call the callback with the response
            request.callback(response);
        }
        else {
            // Call the callback with an empty response to trigger fallback
            request.callback("");
        }
    }
}