#ifndef JWT_MIDDLEWARE_H
#define JWT_MIDDLEWARE_H

#include "crow.h"
#include "domain/entities/user.h"
#include <jwt-cpp/jwt.h>
#include <string>
#include <optional>

namespace web_api {

struct JWTPayload {
    int userId;
    std::string username;
    std::string email;
    UserRole role;
};

class JWTMiddleware {
public:
    explicit JWTMiddleware(const std::string& secret, const std::string& issuer);
    
    // Generate JWT token
    std::string generateToken(const User& user, int expirationHours = 24);
    
    // Validate and decode token
    std::optional<JWTPayload> validateToken(const std::string& token);
    
    // Extract token from Authorization header
    static std::optional<std::string> extractTokenFromHeader(const crow::request& req);
    
    // Check if request has valid token
    bool isAuthenticated(const crow::request& req);
    
    // Check if authenticated user has required role
    bool hasRole(const crow::request& req, UserRole requiredRole);
    
    // Get user ID from request (if authenticated)
    std::optional<int> getUserId(const crow::request& req);

private:
    std::string secret_;
    std::string issuer_;
    
    std::optional<jwt::decoded_jwt<jwt::traits::kazuho_picojson>> decodeToken(const std::string& token);
};

} // namespace web_api

#endif // JWT_MIDDLEWARE_H