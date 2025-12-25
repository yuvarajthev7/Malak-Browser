cmake_minimum_required(VERSION 3.16)
project(MalakBrowser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 COMPONENTS Widgets WebEngineWidgets REQUIRED)

# Add Executable
# "WIN32" hides the console window on Windows (ignored on Linux)
# "WIN32" hides the console window on Windows
# "MACOSX_BUNDLE" creates a .app bundle on macOS
add_executable(malak_browser WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/interceptor.cpp
    src/storage.cpp
    # src/adblock.cpp
)

# Link Qt Libraries
target_link_libraries(malak_browser PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
)
