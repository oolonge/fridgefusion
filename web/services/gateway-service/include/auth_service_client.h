#ifndef AUTH_SERVICE_CLIENT_H
#define AUTH_SERVICE_CLIENT_H

#include <string>
#include <optional>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class AuthServiceClient {
public:
    AuthServiceClient(const std::string& baseUrl);

    // Register new user
    std::optional<json> registerUser(const json& userData);

    // Login user
    std::optional<json> loginUser(const std::string& email, const std::string& password);

    // Verify access token (returns user info if valid)
    std::optional<json> verifyToken(const std::string& token);

    // Refresh access token
    std::optional<json> refreshToken(const std::string& refreshToken);

    // Logout (stateless acknowledgment)
    bool logout();

private:
    std::string baseUrl_;
    std::string doRequest(const std::string& method, const std::string& path, const std::string& body = "");
};

#endif // AUTH_SERVICE_CLIENT_H
