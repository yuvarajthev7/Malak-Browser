cmake_minimum_required(VERSION 3.16)
project(MalakBrowser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets WebEngineWidgets REQUIRED)

add_executable(malak_browser
    src/main.cpp
    src/mainwindow.cpp
    src/interceptor.cpp
    src/storage.cpp      # <--- THIS LINE IS CRITICAL. CHECK IT IS HERE.
)

target_link_libraries(malak_browser PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
)
