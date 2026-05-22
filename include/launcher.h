#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <string>
#include <vector>
#include <memory>

namespace mc_launcher {

struct MinecraftVersion {
    std::string id;
    std::string type;
    std::string url;
    std::string time;
    std::string releaseTime;
};

struct GameProfile {
    std::string username;
    std::string uuid;
    std::vector<std::string> properties;
};

struct LaunchOptions {
    std::string minecraftDir;
    std::string javaPath;
    std::string versionId;
    int minMemory;  // in MB
    int maxMemory;  // in MB
    std::vector<std::string> additionalArgs;
};

class Launcher {
public:
    Launcher();
    ~Launcher();

    // Initialize the launcher with the specified Minecraft directory
    bool initialize(const std::string& minecraftDir);

    // Set Java path manually
    void setJavaPath(const std::string& javaPath);

    // Authenticate user (returns true on success)
    bool authenticate(const std::string& username, const std::string& password = "");

    // Get available versions
    std::vector<MinecraftVersion> getAvailableVersions();

    // Download version manifest
    bool downloadVersionManifest();

    // Download game files for a specific version
    bool downloadGame(const std::string& versionId);

    // Launch the game
    bool launch(const LaunchOptions& options);

    // Check if game is running
    bool isGameRunning() const;

    // Stop the game
    void stopGame();

    // Get last error message
    std::string getLastError() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace mc_launcher

#endif // LAUNCHER_H
