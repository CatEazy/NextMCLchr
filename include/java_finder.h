#ifndef JAVA_FINDER_H
#define JAVA_FINDER_H

#include <string>
#include <vector>
#include <memory>

namespace mc_launcher {

struct JavaInstallation {
    std::string path;
    std::string version;
    int majorVersion;
    bool is64Bit;
};

class JavaFinder {
public:
    JavaFinder();
    ~JavaFinder();

    // Find all Java installations on the system
    std::vector<JavaInstallation> findJavaInstallations();

    // Find the best Java installation automatically
    std::string findBestJavaPath();

    // Check if a specific Java path is valid
    bool isValidJavaPath(const std::string& javaPath);

    // Get Java version from a specific path
    std::string getJavaVersion(const std::string& javaPath);

    // Get the default Java path from environment variables
    std::string getDefaultJavaPath();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
    std::vector<JavaInstallation> searchRegistry();  // Windows only
    std::vector<JavaInstallation> searchCommonPaths();
    bool validateJavaExecutable(const std::string& path);
};

} // namespace mc_launcher

#endif // JAVA_FINDER_H
