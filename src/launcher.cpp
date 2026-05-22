#include "launcher.h"
#include "java_finder.h"
#include "version_manifest.h"
#include "authenticator.h"

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <signal.h>
#endif

namespace mc_launcher {

struct Launcher::Impl {
    std::string minecraftDir;
    std::string javaPath;
    std::unique_ptr<JavaFinder> javaFinder;
    std::unique_ptr<VersionManifestHandler> versionHandler;
    std::unique_ptr<Authenticator> authenticator;
    
    bool gameRunning;
    std::string lastError;
    
#ifdef _WIN32
    HANDLE processHandle;
#else
    pid_t processId;
#endif
    
    Impl() : javaFinder(std::make_unique<JavaFinder>()),
             versionHandler(std::make_unique<VersionManifestHandler>()),
             authenticator(std::make_unique<Authenticator>()),
             gameRunning(false),
             lastError("") {
#ifdef _WIN32
        processHandle = nullptr;
#else
        processId = -1;
#endif
    }
};

Launcher::Launcher() : pImpl(std::make_unique<Impl>()) {}

Launcher::~Launcher() {
    if (isGameRunning()) {
        stopGame();
    }
}

bool Launcher::initialize(const std::string& minecraftDir) {
    pImpl->minecraftDir = minecraftDir;
    
    // Create directory if it doesn't exist
    #ifdef _WIN32
        CreateDirectoryA(minecraftDir.c_str(), nullptr);
    #else
        mkdir(minecraftDir.c_str(), 0755);
    #endif
    
    // Try to find Java automatically
    if (pImpl->javaPath.empty()) {
        pImpl->javaPath = pImpl->javaFinder->findBestJavaPath();
        if (pImpl->javaPath.empty()) {
            pImpl->lastError = "No Java installation found. Please install Java or set the path manually.";
            return false;
        }
    }
    
    return true;
}

void Launcher::setJavaPath(const std::string& javaPath) {
    if (pImpl->javaFinder->isValidJavaPath(javaPath)) {
        pImpl->javaPath = javaPath;
    } else {
        pImpl->lastError = "Invalid Java path: " + javaPath;
    }
}

bool Launcher::authenticate(const std::string& username, const std::string& password) {
    AuthResponse response;
    
    // For demo purposes, we'll use a simple offline mode authentication
    // In a real implementation, you would use Microsoft authentication
    if (password.empty()) {
        // Offline mode - just use the username
        pImpl->lastError = "";
        return true;
    } else {
        // Online mode would go here
        pImpl->lastError = "Online authentication not implemented in this demo.";
        return false;
    }
}

std::vector<MinecraftVersion> Launcher::getAvailableVersions() {
    std::vector<MinecraftVersion> versions;
    
    if (!pImpl->versionHandler->downloadManifest()) {
        pImpl->lastError = pImpl->versionHandler->getLastError();
        return versions;
    }
    
    auto versionInfos = pImpl->versionHandler->getVersions();
    for (const auto& vi : versionInfos) {
        MinecraftVersion mv;
        mv.id = vi.id;
        mv.type = vi.type;
        mv.url = vi.url;
        mv.time = vi.time;
        mv.releaseTime = vi.releaseTime;
        versions.push_back(mv);
    }
    
    return versions;
}

bool Launcher::downloadVersionManifest() {
    return pImpl->versionHandler->downloadManifest();
}

bool Launcher::downloadGame(const std::string& versionId) {
    // Download version details
    std::string versionDir = pImpl->minecraftDir + "/versions/" + versionId;
    
    #ifdef _WIN32
        CreateDirectoryA(versionDir.c_str(), nullptr);
    #else
        mkdir(versionDir.c_str(), 0755);
    #endif
    
    if (!pImpl->versionHandler->downloadVersionDetails(versionId, versionDir)) {
        pImpl->lastError = pImpl->versionHandler->getLastError();
        return false;
    }
    
    // In a full implementation, you would also download:
    // - Game JAR
    // - Libraries
    // - Assets
    
    return true;
}

bool Launcher::launch(const LaunchOptions& options) {
    if (pImpl->gameRunning) {
        pImpl->lastError = "Game is already running.";
        return false;
    }
    
    std::string javaPath = options.javaPath.empty() ? pImpl->javaPath : options.javaPath;
    if (javaPath.empty()) {
        pImpl->lastError = "Java path not set.";
        return false;
    }
    
    // Build command line arguments
    std::string args = "-Xmx" + std::to_string(options.maxMemory) + "M";
    args += " -Xms" + std::to_string(options.minMemory) + "M";
    
    // Add additional arguments
    for (const auto& arg : options.additionalArgs) {
        args += " " + arg;
    }
    
    // Add main class and other Minecraft-specific arguments
    // This is simplified - a real implementation would parse the version JSON
    args += " -cp \"" + pImpl->minecraftDir + "/versions/" + options.versionId + "/" + options.versionId + ".jar\"";
    args += " net.minecraft.client.main.Main";
    args += " --username " + pImpl->authenticator->getUsername();
    args += " --version " + options.versionId;
    args += " --gameDir " + pImpl->minecraftDir;
    args += " --assetsDir " + pImpl->minecraftDir + "/assets";
    args += " --assetIndex " + options.versionId;
    args += " --uuid " + pImpl->authenticator->getUUID();
    
    // Add additional arguments from user
    for (const auto& arg : options.additionalArgs) {
        args += " " + arg;
    }
    
    std::cout << "Launching Minecraft with command:" << std::endl;
    std::cout << javaPath << " " << args << std::endl;
    
    #ifdef _WIN32
        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        
        std::string cmdLine = javaPath + " " + args;
        
        if (CreateProcessA(
                nullptr,
                const_cast<char*>(cmdLine.c_str()),
                nullptr,
                nullptr,
                FALSE,
                0,
                nullptr,
                pImpl->minecraftDir.c_str(),
                &si,
                &pi
            )) {
            pImpl->processHandle = pi.hProcess;
            pImpl->gameRunning = true;
            CloseHandle(pi.hThread);
            return true;
        } else {
            pImpl->lastError = "Failed to create process. Error code: " + std::to_string(GetLastError());
            return false;
        }
    #else
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            chdir(pImpl->minecraftDir.c_str());
            
            // Build argv array
            std::vector<char*> argv;
            argv.push_back(const_cast<char*>(javaPath.c_str()));
            
            // Parse args and add to argv
            std::string argStr = args;
            size_t pos = 0;
            std::string token;
            while ((pos = argStr.find(' ')) != std::string::npos) {
                token = argStr.substr(0, pos);
                if (!token.empty()) {
                    argv.push_back(const_cast<char*>(token.c_str()));
                }
                argStr.erase(0, pos + 1);
            }
            if (!argStr.empty()) {
                argv.push_back(const_cast<char*>(argStr.c_str()));
            }
            argv.push_back(nullptr);
            
            execvp(javaPath.c_str(), argv.data());
            exit(1);  // execvp only returns on error
        } else if (pid > 0) {
            // Parent process
            pImpl->processId = pid;
            pImpl->gameRunning = true;
            return true;
        } else {
            pImpl->lastError = "Failed to fork process.";
            return false;
        }
    #endif
}

bool Launcher::isGameRunning() const {
    if (!pImpl->gameRunning) {
        return false;
    }
    
    #ifdef _WIN32
        if (pImpl->processHandle == nullptr) {
            return false;
        }
        DWORD exitCode;
        if (GetExitCodeProcess(pImpl->processHandle, &exitCode)) {
            if (exitCode == STILL_ACTIVE) {
                return true;
            }
        }
        return false;
    #else
        if (pImpl->processId <= 0) {
            return false;
        }
        int status;
        pid_t result = waitpid(pImpl->processId, &status, WNOHANG);
        if (result == 0) {
            return true;  // Still running
        }
        return false;
    #endif
}

void Launcher::stopGame() {
    if (!pImpl->gameRunning) {
        return;
    }
    
    #ifdef _WIN32
        if (pImpl->processHandle != nullptr) {
            TerminateProcess(pImpl->processHandle, 0);
            CloseHandle(pImpl->processHandle);
            pImpl->processHandle = nullptr;
        }
    #else
        if (pImpl->processId > 0) {
            kill(pImpl->processId, SIGTERM);
            waitpid(pImpl->processId, nullptr, 0);
            pImpl->processId = -1;
        }
    #endif
    
    pImpl->gameRunning = false;
}

std::string Launcher::getLastError() const {
    return pImpl->lastError;
}

} // namespace mc_launcher
