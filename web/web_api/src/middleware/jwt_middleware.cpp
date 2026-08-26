#include "web_api/middleware/jwt_middleware.h"
#include <chrono>

namespace web_api {

JWTMiddleware::JWTMiddleware(const std::string& secret, const std::string& issuer)
    : secret_(secret), issuer_(issuer) {
}

std::string JWTMiddleware::generateToken(const User& user, int expirationHours) {
    auto now = std::chrono::system_clock::now();
    auto expiration = now + std::chrono::hours(expirationHours);
    
    std::string roleStr = (user.getRole() == UserRole::ADMIN) ? "ADMIN" : "USER";
    
    auto token = jwt::create()
        .set_issuer(issuer_)
        .set_type("JWT")
        .set_issued_at(now)
        .set_expires_at(expiration)
        .set_payload_claim("user_id", jwt::claim(std::to_string(user.getId())))
        .set_payload_claim("username", jwt::claim(user.getUsername().toStdString()))
        .set_payload_claim("email", jwt::claim(user.getEmail().toStdString()))
        .set_payload_claim("role", jwt::claim(roleStr))
        .sign(jwt::algorithm::hs256{secret_});
    
    return token;
}

std::optional<JWTPayload> JWTMiddleware::validateToken(const std::string& token) {
    try {
        auto decoded = decodeToken(token);
        if (!decoded.has_value()) {
            return std::nullopt;
        }
        
        auto& jwt = decoded.value();
        
        // Verify token
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_})
            .with_issuer(issuer_);
        
        verifier.verify(jwt);
        
        // Extract payload
        JWTPayload payload;
        payload.userId = std::stoi(jwt.get_payload_claim("user_id").as_string());
        payload.username = jwt.get_payload_claim("username").as_string();
        payload.email = jwt.get_payload_claim("email").as_string();
        
        std::string roleStr = jwt.get_payload_claim("role").as_string();
        payload.role = (roleStr == "ADMIN") ? UserRole::ADMIN : UserRole::USER;
        
        return payload;
        
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

std::optional<std::string> JWTMiddleware::extractTokenFromHeader(const crow::request& req) {
    auto authHeader = req.get_header_value("Authorization");
    if (authHeader.empty()) {
        return std::nullopt;
    }
    
    // Expected format: "Bearer <token>"
    const std::string bearerPrefix = "Bearer ";
    if (authHeader.substr(0, bearerPrefix.length()) != bearerPrefix) {
        return std::nullopt;
    }
    
    return authHeader.substr(bearerPrefix.length());
}

bool JWTMiddleware::isAuthenticated(const crow::request& req) {
    auto token = extractTokenFromHeader(req);
    if (!token.has_value()) {
        return false;
    }
    
    auto payload = validateToken(token.value());
    return payload.has_value();
}

bool JWTMiddleware::hasRole(const crow::request& req, UserRole requiredRole) {
    auto token = extractTokenFromHeader(req);
    if (!token.has_value()) {
        return false;
    }
    
    auto payload = validateToken(token.value());
    if (!payload.has_value()) {
        return false;
    }
    
    // ADMIN has access to everything
    if (payload->role == UserRole::ADMIN) {
        return true;
    }
    
    return payload->role == requiredRole;
}

std::optional<int> JWTMiddleware::getUserId(const crow::request& req) {
    auto token = extractTokenFromHeader(req);
    if (!token.has_value()) {
        return std::nullopt;
    }
    
    auto payload = validateToken(token.value());
    if (!payload.has_value()) {
        return std::nullopt;
    }
    
    return payload->userId;
}

std::optional<jwt::decoded_jwt<jwt::traits::kazuho_picojson>> 
JWTMiddleware::decodeToken(const std::string& token) {
    try {
        return jwt::decode(token);
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

} // namespace web_api