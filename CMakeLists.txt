cmake_minimum_required(VERSION 3.14)
project(TradingPlatform)

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define NOMINMAX and _CRT_SECURE_NO_WARNINGS
add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
# Create fonts directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/fonts)

# Copy fonts directory to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "Copying fonts from ${CMAKE_CURRENT_SOURCE_DIR}/fonts to ${CMAKE_CURRENT_BINARY_DIR}/fonts")

# For Release builds
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/fonts DESTINATION ${CMAKE_INSTALL_PREFIX})

# Include FetchContent for downloading dependencies
include(FetchContent)

# Download ImGui (using docking branch)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# Download ImPlot
FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG v0.16
)
FetchContent_MakeAvailable(implot)

# Create ImGui library with DirectX 11 and Win32 backends
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Create ImPlot library
add_library(implot STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
    ${implot_SOURCE_DIR}/implot_demo.cpp
)

target_include_directories(implot PUBLIC
    ${implot_SOURCE_DIR}
)

# Link ImPlot to ImGui
target_link_libraries(implot PUBLIC imgui)

# Project includes
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${implot_SOURCE_DIR}
)

# Source files
set(SOURCES
    src/main.cpp
    src/App.cpp
    src/TradingUI.cpp
    src/ChartRenderer.cpp
)

set(HEADERS
    include/App.h
    include/TradingUI.h
    include/ChartRenderer.h
    include/main.h
)

# Create executable
add_executable(TradingPlatform WIN32 ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(TradingPlatform PRIVATE
    imgui
    implot
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
    user32.lib
    gdi32.lib
    shell32.lib
)

# MSVC specific settings
if(MSVC)
    # Important: Define ImDrawIdx as unsigned int (32-bit indices)
    # This is important for ImPlot to work correctly with high density plots
    add_definitions(-DImDrawIdx=unsigned\ int)
    
    set_property(TARGET TradingPlatform PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    set_target_properties(TradingPlatform PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})
endif()