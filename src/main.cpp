#include "launcher.h"
#include "java_finder.h"
#include "version_manifest.h"
#include "authenticator.h"

#include <iostream>
#include <string>
#include <memory>

using namespace mc_launcher;

void printBanner() {
    std::cout << "========================================" << std::endl;
    std::cout << "     C++ Minecraft Launcher v1.0.0      " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}

void printMenu() {
    std::cout << "\n--- Main Menu ---" << std::endl;
    std::cout << "1. Authenticate (Offline Mode)" << std::endl;
    std::cout << "2. List Available Versions" << std::endl;
    std::cout << "3. Download Game Version" << std::endl;
    std::cout << "4. Launch Game" << std::endl;
    std::cout << "5. Check Java Installation" << std::endl;
    std::cout << "6. Exit" << std::endl;
    std::cout << "> ";
}

int main(int argc, char* argv[]) {
    printBanner();
    
    // Create launcher instance
    auto launcher = std::make_unique<Launcher>();
    
    // Set default Minecraft directory
    std::string minecraftDir = "./minecraft";
    
    // Initialize launcher
    std::cout << "Initializing launcher..." << std::endl;
    if (!launcher->initialize(minecraftDir)) {
        std::cerr << "Failed to initialize: " << launcher->getLastError() << std::endl;
        return 1;
    }
    std::cout << "Launcher initialized successfully!" << std::endl;
    std::cout << "Minecraft directory: " << minecraftDir << std::endl;
    
    // Main menu loop
    int choice;
    std::string username;
    std::string versionId;
    
    do {
        printMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1: {
                // Authenticate
                std::cout << "Enter username for offline mode: ";
                std::cin >> username;
                
                if (launcher->authenticate(username)) {
                    std::cout << "Authentication successful!" << std::endl;
                } else {
                    std::cerr << "Authentication failed: " << launcher->getLastError() << std::endl;
                }
                break;
            }
            
            case 2: {
                // List available versions
                std::cout << "Fetching available versions..." << std::endl;
                auto versions = launcher->getAvailableVersions();
                
                if (versions.empty()) {
                    std::cout << "No versions available or failed to fetch." << std::endl;
                } else {
                    std::cout << "\nAvailable Versions:" << std::endl;
                    std::cout << "-------------------" << std::endl;
                    for (const auto& v : versions) {
                        std::cout << "[" << v.type << "] " << v.id 
                                  << " (" << v.releaseTime.substr(0, 10) << ")" << std::endl;
                    }
                }
                break;
            }
            
            case 3: {
                // Download game version
                std::cout << "Enter version ID to download (e.g., 1.20.4): ";
                std::cin >> versionId;
                
                std::cout << "Downloading version " << versionId << "..." << std::endl;
                if (launcher->downloadGame(versionId)) {
                    std::cout << "Download completed successfully!" << std::endl;
                } else {
                    std::cerr << "Download failed: " << launcher->getLastError() << std::endl;
                }
                break;
            }
            
            case 4: {
                // Launch game
                LaunchOptions options;
                options.minecraftDir = minecraftDir;
                options.versionId = "1.20.4";  // Default version
                
                std::cout << "Enter version to launch (default: 1.20.4): ";
                std::cin >> versionId;
                if (!versionId.empty()) {
                    options.versionId = versionId;
                }
                
                options.minMemory = 512;   // 512 MB minimum
                options.maxMemory = 2048;  // 2 GB maximum
                
                std::cout << "Launching Minecraft version " << options.versionId << "..." << std::endl;
                std::cout << "Memory allocation: " << options.minMemory << "MB - " 
                          << options.maxMemory << "MB" << std::endl;
                
                if (launcher->launch(options)) {
                    std::cout << "Game launched successfully!" << std::endl;
                    std::cout << "Note: This is a demo - actual game launch requires complete implementation." << std::endl;
                } else {
                    std::cerr << "Failed to launch: " << launcher->getLastError() << std::endl;
                }
                break;
            }
            
            case 5: {
                // Check Java installation
                JavaFinder finder;
                auto installations = finder.findJavaInstallations();
                
                if (installations.empty()) {
                    std::cout << "No Java installations found." << std::endl;
                } else {
                    std::cout << "\nFound Java installations:" << std::endl;
                    std::cout << "-------------------------" << std::endl;
                    for (const auto& inst : installations) {
                        std::cout << "Path: " << inst.path << std::endl;
                        std::cout << "  Version: " << inst.version << std::endl;
                        std::cout << "  64-bit: " << (inst.is64Bit ? "Yes" : "No") << std::endl;
                        std::cout << std::endl;
                    }
                    
                    std::string bestPath = finder.findBestJavaPath();
                    if (!bestPath.empty()) {
                        std::cout << "Best Java path: " << bestPath << std::endl;
                    }
                }
                break;
            }
            
            case 6:
                std::cout << "Exiting..." << std::endl;
                break;
                
            default:
                std::cout << "Invalid option. Please try again." << std::endl;
        }
        
    } while (choice != 6);
    
    std::cout << "\nThank you for using C++ Minecraft Launcher!" << std::endl;
    return 0;
}
