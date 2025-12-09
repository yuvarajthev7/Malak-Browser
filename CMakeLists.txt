cmake_minimum_required(VERSION 3.17)
project(MalakBrowser)

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(WEBKIT REQUIRED webkitgtk-6.0)

# Build Configuration
set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(src)

# Source Files
set(SOURCES
    src/main.cpp
    src/browser.cpp
    src/adblock.cpp
    src/storage.cpp
)

# Executable
add_executable(malak_browser ${SOURCES})

# Linking
target_link_libraries(malak_browser PRIVATE ${GTK4_LIBRARIES} ${WEBKIT_LIBRARIES})
target_include_directories(malak_browser PRIVATE ${GTK4_INCLUDE_DIRS} ${WEBKIT_INCLUDE_DIRS})
target_compile_options(malak_browser PRIVATE ${GTK4_CFLAGS} ${WEBKIT_CFLAGS})
