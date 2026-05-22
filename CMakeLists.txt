cmake_minimum_required(VERSION 3.10)
project(MCLauncher VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/launcher.cpp
    src/java_finder.cpp
    src/version_manifest.cpp
    src/authenticator.cpp
)

# Header files
set(HEADERS
    include/launcher.h
    include/java_finder.h
    include/version_manifest.h
    include/authenticator.h
)

# Create executable
add_executable(mc_launcher ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(mc_launcher 
    nlohmann_json::nlohmann_json
    CURL::libcurl
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(mc_launcher PRIVATE _WINDOWS)
    # On Windows, you might need to link additional libraries for process creation
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(mc_launcher PRIVATE _LINUX)
elseif(APPLE)
    target_compile_definitions(mc_launcher PRIVATE _MACOS)
endif()

# Install target
install(TARGETS mc_launcher DESTINATION bin)
