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

bool CryptoAPIClient::FetchLatestQuote(const std::string& symbol, std::function<void(const PriceData&, bool isRealData)> callback) {
    // Enqueue the API request
    std::function<void(const std::string&)> responseCallback = [callback, symbol, this](const std::string& response) {
        try {
            // If response is empty, use mock data
            if (response.empty()) {
                PriceData mockData = GenerateMockPriceData(symbol);
                callback(mockData, false); // false indicates mock data
                return;
            }

            // Parse JSON response
            nlohmann::json json = nlohmann::json::parse(response);

            // Handle API error response
            if (json.contains("status") && json["status"].contains("error_code") && json["status"]["error_code"] != 0) {
                m_lastError = "API Error: " + json["status"]["error_message"].get<std::string>();
                std::cerr << m_lastError << std::endl;

                // Fallback to mock data
                PriceData mockData = GenerateMockPriceData(symbol);
                callback(mockData, false); // false indicates mock data
                return;
            }

            // Parse the JSON response based on CoinMarketCap's API structure
            if (json.contains("data") && json["data"].contains(symbol)) {
                PriceData priceData;
                priceData.symbol = symbol;

                // Handle the data structure, which might be different than our simplified expectations
                try {
                    const auto& symbolData = json["data"][symbol];
                    // Check if it's an array or object
                    const auto& data = symbolData.is_array() ? symbolData[0] : symbolData;
                    const auto& quote = data["quote"]["USD"];

                    priceData.price = quote["price"].get<double>();
                    priceData.volume24h = quote["volume_24h"].get<double>();

                    // These fields might not exist in all API response tiers
                    if (quote.contains("percent_change_1h"))
                        priceData.percentChange1h = quote["percent_change_1h"].get<double>();
                    if (quote.contains("percent_change_24h"))
                        priceData.percentChange24h = quote["percent_change_24h"].get<double>();
                    if (quote.contains("percent_change_7d"))
                        priceData.percentChange7d = quote["percent_change_7d"].get<double>();
                    if (quote.contains("market_cap"))
                        priceData.marketCap = quote["market_cap"].get<double>();

                    priceData.lastUpdated = quote["last_updated"].get<std::string>();

                    // Copy price to other fields for chart consistency
                    priceData.open = priceData.price;
                    priceData.high = priceData.price;
                    priceData.low = priceData.price;
                    priceData.close = priceData.price;

                    // Debug output to confirm API response
                    std::cout << "Successfully parsed real API data for " << symbol << ": $"
                        << priceData.price << std::endl;

                    callback(priceData, true); // true indicates real API data
                    return;
                }
                catch (const std::exception& e) {
                    // Specific parsing errors in data structure, fallback to mock
                    std::cerr << "Error parsing data structure: " << e.what() << std::endl;
                }
            }

            // If we reach here, either data structure wasn't as expected or parsing failed
            // Use mock data as fallback
            PriceData mockData = GenerateMockPriceData(symbol);
            callback(mockData, false); // false indicates mock data

        }
        catch (const std::exception& e) {
            // Handle general parsing errors
            std::cerr << "Error parsing API response: " << e.what() << std::endl;
            m_lastError = std::string("Parsing error: ") + e.what();

            // Fallback to mock data
            PriceData mockData = GenerateMockPriceData(symbol);
            callback(mockData, false); // false indicates mock data
        }
        };

    // Enqueue the request
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_requestQueue.push_back({ "/v2/cryptocurrency/quotes/latest", {{"symbol", symbol}, {"convert", "USD"}}, responseCallback });
    }
    m_queueCondition.notify_one();

    return true;
}

PriceData CryptoAPIClient::GenerateMockPriceData(const std::string& symbol) {
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

    return mockData;
}

bool CryptoAPIClient::FetchHistoricalData(const std::string& symbol, std::function<void(const std::vector<PriceData>&)> callback) {
    // CoinMarketCap doesn't have a free historical data endpoint in their API
    // So we'll generate mock historical data for the chart
    std::vector<PriceData> historicalData;
    GenerateMockHistoricalData(symbol, historicalData);

    // Call the callback with the mock data
    callback(historicalData);

    return true;
}

void CryptoAPIClient::GenerateMockHistoricalData(const std::string& symbol, std::vector<PriceData>& data, int numDays) {
    // Seed for random generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

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