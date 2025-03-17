# Trading Platform

A modern cryptocurrency trading platform interface built with Dear ImGui and DirectX 11.

## Overview

This project is a desktop trading platform for cryptocurrency with a professional UI featuring real-time price updates, trading panels, and position management. It's designed to provide a responsive and customizable trading experience.

## Technologies Used

- C++17
- Dear ImGui (Docking branch)
- DirectX 11 for rendering
- CMake for build configuration

## Features

- **Responsive UI** with dockable windows and dynamic layout
- **Trading Panel** with:
  - Buy/Sell tabs
  - Multiple order types (Market, Limit, Stop, Stop Limit)
  - Percentage-based amount selection
  - Real-time price display with animations
  - Fee calculation
- **Positions Window** for tracking open trades
- **Chart Window** (placeholder for future implementation)
- **Dark Theme** with modern styling

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

