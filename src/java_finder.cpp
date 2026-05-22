#include "java_finder.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <dirent.h>
#endif

namespace mc_launcher {

struct JavaFinder::Impl {
    std::vector<std::string> commonJavaPaths;
    
    Impl() {
        // Initialize common Java paths based on platform
        #ifdef _WIN32
            commonJavaPaths = {
                "C:\\Program Files\\Java\\",
                "C:\\Program Files (x86)\\Java\\",
                "C:\\Program Files\\Eclipse Adoptium\\",
                "C:\\Program Files\\Microsoft\\jdk-",
            };
        #elif defined(__APPLE__)
            commonJavaPaths = {
                "/Library/Java/JavaVirtualMachines/",
                "/usr/bin/java",
                "/usr/libexec/java_home",
            };
        #else
            commonJavaPaths = {
                "/usr/lib/jvm/",
                "/usr/bin/java",
                "/opt/java/",
                "/usr/local/java/",
            };
        #endif
    }
};

JavaFinder::JavaFinder() : pImpl(std::make_unique<Impl>()) {}

JavaFinder::~JavaFinder() = default;

std::vector<JavaInstallation> JavaFinder::findJavaInstallations() {
    std::vector<JavaInstallation> installations;
    
    // Search common paths
    auto commonInstallations = searchCommonPaths();
    installations.insert(installations.end(), commonInstallations.begin(), commonInstallations.end());
    
    // On Windows, also search registry
    #ifdef _WIN32
        auto registryInstallations = searchRegistry();
        installations.insert(installations.end(), registryInstallations.begin(), registryInstallations.end());
    #endif
    
    // Remove duplicates
    std::sort(installations.begin(), installations.end(), 
              [](const JavaInstallation& a, const JavaInstallation& b) {
                  return a.path < b.path;
              });
    installations.erase(std::unique(installations.begin(), installations.end(),
              [](const JavaInstallation& a, const JavaInstallation& b) {
                  return a.path == b.path;
              }), installations.end());
    
    return installations;
}

std::string JavaFinder::findBestJavaPath() {
    // First, check environment variables
    std::string envPath = getDefaultJavaPath();
    if (!envPath.empty() && isValidJavaPath(envPath)) {
        return envPath;
    }
    
    // Find all installations
    auto installations = findJavaInstallations();
    
    if (installations.empty()) {
        return "";
    }
    
    // Prefer 64-bit Java with higher version
    std::sort(installations.begin(), installations.end(),
              [](const JavaInstallation& a, const JavaInstallation& b) {
                  if (a.is64Bit != b.is64Bit) {
                      return a.is64Bit > b.is64Bit;  // Prefer 64-bit
                  }
                  return a.majorVersion > b.majorVersion;  // Prefer higher version
              });
    
    return installations[0].path;
}

bool JavaFinder::isValidJavaPath(const std::string& javaPath) {
    #ifdef _WIN32
        struct stat buffer;
        if (stat(javaPath.c_str(), &buffer) != 0) {
            return false;
        }
        return true;
    #else
        return access(javaPath.c_str(), X_OK) == 0;
    #endif
}

std::string JavaFinder::getJavaVersion(const std::string& javaPath) {
    // In a real implementation, you would execute "javaPath -version" and parse the output
    // For this demo, we'll return a placeholder
    return "1.8.0";
}

std::string JavaFinder::getDefaultJavaPath() {
    // Check JAVA_HOME environment variable
    const char* javaHome = std::getenv("JAVA_HOME");
    if (javaHome != nullptr && strlen(javaHome) > 0) {
        std::string path = std::string(javaHome);
        #ifdef _WIN32
            path += "\\bin\\java.exe";
        #else
            path += "/bin/java";
        #endif
        if (isValidJavaPath(path)) {
            return path;
        }
    }
    
    // Check PATH for java executable
    const char* pathEnv = std::getenv("PATH");
    if (pathEnv != nullptr) {
        std::istringstream iss(pathEnv);
        std::string dir;
        while (std::getline(iss, dir, 
            #ifdef _WIN32
                ';'
            #else
                ':'
            #endif
        )) {
            std::string javaExe = dir + 
                #ifdef _WIN32
                    "\\java.exe"
                #else
                    "/java"
                #endif
            ;
            if (isValidJavaPath(javaExe)) {
                return javaExe;
            }
        }
    }
    
    return "";
}

std::vector<JavaInstallation> JavaFinder::searchRegistry() {
    // Windows-specific registry search
    std::vector<JavaInstallation> installations;
    
    #ifdef _WIN32
        HKEY hKey;
        // Search in HKLM\SOFTWARE\JavaSoft\JRE
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\JavaSoft\\JRE", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            // This is simplified - a real implementation would enumerate subkeys
            RegCloseKey(hKey);
        }
        
        // Search in HKLM\SOFTWARE\JavaSoft\JDK
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\JavaSoft\\JDK", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
        }
    #endif
    
    return installations;
}

std::vector<JavaInstallation> JavaFinder::searchCommonPaths() {
    std::vector<JavaInstallation> installations;
    
    for (const auto& basePath : pImpl->commonJavaPaths) {
        #ifdef _WIN32
            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA((basePath + "*").c_str(), &findData);
            
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        std::string dirName = findData.cFileName;
                        if (dirName == "." || dirName == "..") continue;
                        
                        std::string javaPath = basePath + dirName + "\\bin\\java.exe";
                        if (isValidJavaPath(javaPath)) {
                            JavaInstallation inst;
                            inst.path = javaPath;
                            inst.version = getJavaVersion(javaPath);
                            inst.majorVersion = 8;  // Simplified
                            inst.is64Bit = true;     // Simplified
                            installations.push_back(inst);
                        }
                    }
                } while (FindNextFileA(hFind, &findData));
                FindClose(hFind);
            }
        #else
            DIR* dir = opendir(basePath.c_str());
            if (dir != nullptr) {
                struct dirent* entry;
                while ((entry = readdir(dir)) != nullptr) {
                    if (entry->d_type == DT_DIR) {
                        std::string dirName = entry->d_name;
                        if (dirName == "." || dirName == "..") continue;
                        
                        std::string javaPath = basePath + dirName + "/bin/java";
                        if (isValidJavaPath(javaPath)) {
                            JavaInstallation inst;
                            inst.path = javaPath;
                            inst.version = getJavaVersion(javaPath);
                            inst.majorVersion = 8;  // Simplified
                            inst.is64Bit = true;     // Simplified
                            installations.push_back(inst);
                        }
                    }
                }
                closedir(dir);
            }
            
            // Also check if /usr/bin/java exists
            if (basePath == "/usr/bin/java" && isValidJavaPath("/usr/bin/java")) {
                JavaInstallation inst;
                inst.path = "/usr/bin/java";
                inst.version = getJavaVersion("/usr/bin/java");
                inst.majorVersion = 8;
                inst.is64Bit = true;
                installations.push_back(inst);
            }
        #endif
    }
    
    return installations;
}

bool JavaFinder::validateJavaExecutable(const std::string& path) {
    return isValidJavaPath(path);
}

} // namespace mc_launcher
