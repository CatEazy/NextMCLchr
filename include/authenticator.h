#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <string>
#include <map>
#include <vector>

namespace mc_launcher {

struct AuthResponse {
    bool success;
    std::string accessToken;
    std::string clientToken;
    std::string uuid;
    std::string username;
    std::string errorMessage;
    std::vector<std::string> availableProfiles;
};

class Authenticator {
public:
    Authenticator();
    ~Authenticator();

    // Authenticate with Microsoft account (modern authentication)
    AuthResponse authenticateMicrosoft(const std::string& email);

    // Authenticate with Mojang account (legacy, deprecated)
    AuthResponse authenticateMojang(const std::string& username, const std::string& password);

    // Refresh access token
    AuthResponse refreshAccessToken(const std::string& accessToken);

    // Validate current access token
    bool validateAccessToken(const std::string& accessToken);

    // Sign out (invalidate tokens)
    bool signOut(const std::string& accessToken);

    // Get authentication status
    bool isAuthenticated() const;

    // Get current access token
    std::string getAccessToken() const;

    // Get current username
    std::string getUsername() const;

    // Get current UUID
    std::string getUUID() const;

    // Clear stored credentials
    void clearCredentials();

    // Get last error message
    std::string getLastError() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace mc_launcher

#endif // AUTHENTICATOR_H
