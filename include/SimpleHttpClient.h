#pragma once

#include <string>
#include <map>
#include <functional>
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

class SimpleHttpClient {
public:
    // Make a GET request to a URL with custom headers
    static bool Get(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        std::string& response,
        std::string& error
    ) {
        // Convert URL to wide string
        std::wstring wideUrl = StringToWideString(url);

        // Parse URL components
        URL_COMPONENTS urlComp = { 0 };
        urlComp.dwStructSize = sizeof(urlComp);

        wchar_t hostName[256] = { 0 };
        urlComp.lpszHostName = hostName;
        urlComp.dwHostNameLength = sizeof(hostName) / sizeof(hostName[0]);

        wchar_t urlPath[2048] = { 0 };
        urlComp.lpszUrlPath = urlPath;
        urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(urlPath[0]);

        BOOL result = WinHttpCrackUrl(wideUrl.c_str(), (DWORD)wideUrl.length(), 0, &urlComp);
        if (!result) {
            error = "Failed to parse URL: " + GetLastErrorAsString();
            return false;
        }

        // Determine if HTTPS
        bool isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

        // Initialize WinHttp
        HINTERNET hSession = WinHttpOpen(
            L"TradingPlatform/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0
        );

        if (!hSession) {
            error = "Failed to initialize WinHttp: " + GetLastErrorAsString();
            return false;
        }

        // Connect to server
        HINTERNET hConnect = WinHttpConnect(
            hSession,
            hostName,
            urlComp.nPort,
            0
        );

        if (!hConnect) {
            error = "Failed to connect to server: " + GetLastErrorAsString();
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Create request
        HINTERNET hRequest = WinHttpOpenRequest(
            hConnect,
            L"GET",
            urlPath,
            NULL,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            isHttps ? WINHTTP_FLAG_SECURE : 0
        );

        if (!hRequest) {
            error = "Failed to create request: " + GetLastErrorAsString();
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Set request timeout (10 seconds)
        DWORD timeout = 10000;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(hRequest, WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

        // Add headers
        for (const auto& header : headers) {
            std::wstring headerStr = StringToWideString(header.first + ": " + header.second);
            WinHttpAddRequestHeaders(
                hRequest,
                headerStr.c_str(),
                (DWORD)headerStr.length(),
                WINHTTP_ADDREQ_FLAG_ADD
            );
        }

        // Send request
        result = WinHttpSendRequest(
            hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            WINHTTP_NO_REQUEST_DATA,
            0,
            0,
            0
        );

        if (!result) {
            error = "Failed to send request: " + GetLastErrorAsString();
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Receive response
        result = WinHttpReceiveResponse(hRequest, NULL);
        if (!result) {
            error = "Failed to receive response: " + GetLastErrorAsString();
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Check HTTP status code
        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        WinHttpQueryHeaders(
            hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &statusCode,
            &statusCodeSize,
            WINHTTP_NO_HEADER_INDEX
        );

        if (statusCode != 200) {
            error = "HTTP error: " + std::to_string(statusCode);
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Read response data
        response.clear();
        DWORD bytesAvailable = 0;
        DWORD bytesRead = 0;
        char buffer[4096] = { 0 };

        do {
            bytesAvailable = 0;
            result = WinHttpQueryDataAvailable(hRequest, &bytesAvailable);

            if (!result || bytesAvailable == 0) {
                break;
            }

            ZeroMemory(buffer, sizeof(buffer));

            if (WinHttpReadData(
                hRequest,
                buffer,
                sizeof(buffer) - 1,
                &bytesRead
            )) {
                if (bytesRead > 0) {
                    response.append(buffer, bytesRead);
                }
            }
        } while (bytesAvailable > 0);

        // Clean up
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        return true;
    }

private:
    // Utility function to convert string to wide string
    static std::wstring StringToWideString(const std::string& str) {
        if (str.empty()) {
            return std::wstring();
        }

        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &result[0], size);
        return result;
    }

    // Utility function to get the last error as string
    static std::string GetLastErrorAsString() {
        DWORD error = GetLastError();
        if (error == 0) {
            return std::string();
        }

        LPSTR buffer = nullptr;
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&buffer,
            0,
            NULL
        );

        std::string message(buffer, size);
        LocalFree(buffer);

        return "Error " + std::to_string(error) + ": " + message;
    }
};