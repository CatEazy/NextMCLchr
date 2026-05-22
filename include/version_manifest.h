#ifndef VERSION_MANIFEST_H
#define VERSION_MANIFEST_H

#include <string>
#include <vector>
#include <map>

namespace mc_launcher {

struct VersionInfo {
    std::string id;
    std::string type;  // "release", "snapshot", "old_beta", "old_alpha"
    std::string url;
    std::string time;
    std::string releaseTime;
};

struct VersionManifest {
    std::string latest_release;
    std::string latest_snapshot;
    std::vector<VersionInfo> versions;
};

struct AssetIndex {
    std::string id;
    std::string sha1;
    int size;
    int totalSize;
    std::string url;
};

struct Library {
    std::string name;
    std::string url;
    std::vector<std::string> rules;
    std::map<std::string, std::string> natives;
};

struct VersionDetails {
    std::string id;
    std::string type;
    std::string minecraftArguments;
    std::string mainClass;
    std::string inheritsFrom;
    std::string jar;
    AssetIndex assetIndex;
    std::string assets;
    std::vector<Library> libraries;
    std::map<std::string, std::string> downloads;  // client, server mappings
};

class VersionManifestHandler {
public:
    VersionManifestHandler();
    ~VersionManifestHandler();

    // Download the version manifest from Mojang
    bool downloadManifest(const std::string& manifestUrl = "https://launchermeta.mojang.com/mc/game/version_manifest.json");

    // Parse the manifest JSON
    bool parseManifest(const std::string& jsonContent);

    // Get all available versions
    std::vector<VersionInfo> getVersions() const;

    // Get version by ID
    VersionInfo getVersionById(const std::string& id) const;

    // Get latest release version
    std::string getLatestRelease() const;

    // Get latest snapshot version
    std::string getLatestSnapshot() const;

    // Download version details JSON
    bool downloadVersionDetails(const std::string& versionId, const std::string& outputDir);

    // Parse version details
    bool parseVersionDetails(const std::string& jsonContent, VersionDetails& details);

    // Download game JAR
    bool downloadGameJar(const std::string& url, const std::string& outputPath);

    // Download libraries
    bool downloadLibraries(const std::vector<Library>& libraries, const std::string& outputDir);

    // Download assets
    bool downloadAssets(const AssetIndex& assetIndex, const std::string& outputDir);

    // Get last error
    std::string getLastError() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace mc_launcher

#endif // VERSION_MANIFEST_H
