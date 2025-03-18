#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

// Structure to store price data from the API
struct PriceData {
    std::string symbol;
    double price = 0.0;
    double volume24h = 0.0;
    double percentChange1h = 0.0;
    double percentChange24h = 0.0;
    double percentChange7d = 0.0;
    double marketCap = 0.0;
    std::string lastUpdated;

    // For candlestick data
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    double volume = 0.0;
    double timestamp = 0.0;
};

// Class for handling API communication with CoinMarketCap
class CryptoAPIClient {
public:
    CryptoAPIClient();
    ~CryptoAPIClient();

    // Initialize the API client with API key
    bool Initialize(const std::string& apiKey);

    // Shutdown the API client
    void Shutdown();

    // Fetch latest quote for a cryptocurrency
    bool FetchLatestQuote(const std::string& symbol, std::function<void(const PriceData&)> callback);

    // Fetch historical data for a cryptocurrency (for charts)
    bool FetchHistoricalData(const std::string& symbol, std::function<void(const std::vector<PriceData>&)> callback);

    // Get the error message
    const std::string& GetLastError() const { return m_lastError; }

private:
    // API key
    std::string m_apiKey;

    // Base URL for API requests 
    std::string m_baseUrl;

    // Last error message
    std::string m_lastError;

    // Helper method to make an API request
    bool MakeRequest(const std::string& endpoint, const std::map<std::string, std::string>& params, std::string& response);

    // Thread for handling API requests in the background
    std::unique_ptr<std::thread> m_requestThread;

    // Flag to indicate if the thread should stop
    std::atomic<bool> m_shouldStop;

    // Queue of API requests
    struct APIRequest {
        std::string endpoint;
        std::map<std::string, std::string> params;
        std::function<void(const std::string&)> callback;
    };

    std::vector<APIRequest> m_requestQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCondition;

    // Thread function for processing requests
    void ProcessRequests();

    // Generate mock historical data for demonstration
    void GenerateMockHistoricalData(const std::string& symbol, std::vector<PriceData>& data, int numDays = 100);
};