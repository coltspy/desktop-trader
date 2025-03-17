#pragma once

#include <Windows.h>
#include <memory>
#include <string>
#include <iostream>

// Forward declarations
class App;

// Global constants
constexpr const wchar_t* APP_TITLE = L"Trading Platform";
constexpr const wchar_t* WINDOW_CLASS_NAME = L"TradingPlatform";

// Window procedure declaration
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);