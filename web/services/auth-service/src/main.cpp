#include "crow.h"
#include "nlohmann/json.hpp"
#include "http_client.h"
#include "jwt-cpp/jwt.h"
#include <memory>
#include <iostream>
#include <cstdlib>
#include <chrono>

using json = nlohmann::json;

// JWT configuration
struct JWTConfig {
    std::string secret;
    std::string issuer;
    int expirationHours;
    int refreshExpirationDays;
};

// Generate access token
std::string generateAccessToken(const json& user, const JWTConfig& config) {
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::hours(config.expirationHours);

    auto token = jwt::create()
        .set_issuer(config.issuer)
        .set_type("JWT")
        .set_issued_at(now)
        .set_expires_at(exp)
        .set_payload_claim("user_id", jwt::claim(std::to_string(user["id"].get<int>())))
        .set_payload_claim("email", jwt::claim(user["email"].get<std::string>()))
        .set_payload_claim("role", jwt::claim(std::to_string(user["role"].get<int>())))
        .set_payload_claim("token_type", jwt::claim(std::string("access")))
        .sign(jwt::algorithm::hs256{config.secret});

    return token;
}

// Generate refresh token
std::string generateRefreshToken(const json& user, const JWTConfig& config) {
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::hours(config.refreshExpirationDays * 24);

    auto token = jwt::create()
        .set_issuer(config.issuer)
        .set_type("JWT")
        .set_issued_at(now)
        .set_expires_at(exp)
        .set_payload_claim("user_id", jwt::claim(std::to_string(user["id"].get<int>())))
        .set_payload_claim("token_type", jwt::claim(std::string("refresh")))
        .sign(jwt::algorithm::hs256{config.secret});

    return token;
}

// Verify token and extract claims
std::optional<json> verifyToken(const std::string& token, const JWTConfig& config, const std::string& expectedType = "") {
    try {
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{config.secret})
            .with_issuer(config.issuer);

        auto decoded = jwt::decode(token);
        verifier.verify(decoded);

        // Check token type if specified
        if (!expectedType.empty()) {
            auto tokenType = decoded.get_payload_claim("token_type").as_string();
            if (tokenType != expectedType) {
                std::cerr << "Invalid token type: expected " << expectedType << ", got " << tokenType << std::endl;
                return std::nullopt;
            }
        }

        json claims;
        claims["user_id"] = std::stoi(decoded.get_payload_claim("user_id").as_string());

        if (decoded.has_payload_claim("email")) {
            claims["email"] = decoded.get_payload_claim("email").as_string();
        }
        if (decoded.has_payload_claim("role")) {
            claims["role"] = std::stoi(decoded.get_payload_claim("role").as_string());
        }
        claims["token_type"] = decoded.get_payload_claim("token_type").as_string();

        return claims;
    } catch (const std::exception& e) {
        std::cerr << "JWT verification failed: " << e.what() << std::endl;
        return std::nullopt;
    }
}

int main() {
    std::cout << "Starting Auth Service..." << std::endl;

    // Get configuration from environment
    const char* coreServiceUrl = std::getenv("CORE_SERVICE_URL");
    const char* portStr = std::getenv("PORT");
    const char* jwtSecret = std::getenv("JWT_SECRET");
    const char* jwtIssuer = std::getenv("JWT_ISSUER");
    const char* jwtExpStr = std::getenv("JWT_EXPIRATION_HOURS");
    const char* refreshExpStr = std::getenv("JWT_REFRESH_EXPIRATION_DAYS");

    std::string coreUrl = coreServiceUrl ? coreServiceUrl : "http://localhost:8081";
    int port = portStr ? std::stoi(portStr) : 8083;

    JWTConfig jwtConfig;
    jwtConfig.secret = jwtSecret ? jwtSecret : "your-secret-key-change-in-production";
    jwtConfig.issuer = jwtIssuer ? jwtIssuer : "FridgeFusion";
    jwtConfig.expirationHours = jwtExpStr ? std::stoi(jwtExpStr) : 24;
    jwtConfig.refreshExpirationDays = refreshExpStr ? std::stoi(refreshExpStr) : 7;

    auto coreClient = std::make_shared<HttpClient>(coreUrl);

    crow::SimpleApp app;

    // ==================== HEALTH CHECK ====================
    CROW_ROUTE(app, "/auth/health")
    ([]() {
        json response = {
            {"status", "ok"},
            {"service", "auth-service"},
            {"version", "1.0.0"}
        };
        return crow::response(200, response.dump());
    });

    // ==================== AUTH ROUTES ====================

    // POST /auth/register - Register new user
    CROW_ROUTE(app, "/auth/register").methods("POST"_method)
    ([&coreClient, &jwtConfig](const crow::request& req) {
        try {
            auto body = json::parse(req.body);

            // Call Core Service to register user
            auto result = coreClient->post("/api/core/auth/register", body);

            if (result && result->contains("id")) {
                // Generate tokens for new user
                json user = *result;
                std::string accessToken = generateAccessToken(user, jwtConfig);
                std::string refreshToken = generateRefreshToken(user, jwtConfig);

                json response = {
                    {"access_token", accessToken},
                    {"refresh_token", refreshToken},
                    {"token_type", "Bearer"},
                    {"expires_in", jwtConfig.expirationHours * 3600},
                    {"user", user}
                };
                return crow::response(201, response.dump());
            }

            // Return error from Core Service
            if (result && result->contains("error")) {
                return crow::response(400, result->dump());
            }

            return crow::response(500, R"({"error":"Registration failed"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // POST /auth/login - Authenticate user
    CROW_ROUTE(app, "/auth/login").methods("POST"_method)
    ([&coreClient, &jwtConfig](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            std::string email = body["email"].get<std::string>();
            std::string password = body["password"].get<std::string>();

            // Call Core Service to verify credentials
            json loginRequest = {
                {"email", email},
                {"password", password}
            };
            auto result = coreClient->post("/api/core/auth/login", loginRequest);

            if (result && result->contains("success") && (*result)["success"].get<bool>()) {
                auto user = (*result)["user"];
                std::string accessToken = generateAccessToken(user, jwtConfig);
                std::string refreshToken = generateRefreshToken(user, jwtConfig);

                json response = {
                    {"access_token", accessToken},
                    {"refresh_token", refreshToken},
                    {"token_type", "Bearer"},
                    {"expires_in", jwtConfig.expirationHours * 3600},
                    {"user", user}
                };
                return crow::response(200, response.dump());
            }

            return crow::response(401, R"({"error":"Invalid credentials"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // POST /auth/verify - Verify access token (internal use by Gateway)
    CROW_ROUTE(app, "/auth/verify").methods("POST"_method)
    ([&jwtConfig](const crow::request& req) {
        try {
            auto body = json::parse(req.body);

            if (!body.contains("token")) {
                return crow::response(400, R"({"error":"Token required"})");
            }

            std::string token = body["token"].get<std::string>();
            auto claims = verifyToken(token, jwtConfig, "access");

            if (claims) {
                json response = {
                    {"valid", true},
                    {"user", *claims}
                };
                return crow::response(200, response.dump());
            }

            json response = {
                {"valid", false},
                {"error", "Invalid or expired token"}
            };
            return crow::response(401, response.dump());
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // POST /auth/refresh - Refresh access token
    CROW_ROUTE(app, "/auth/refresh").methods("POST"_method)
    ([&coreClient, &jwtConfig](const crow::request& req) {
        try {
            auto body = json::parse(req.body);

            if (!body.contains("refresh_token")) {
                return crow::response(400, R"({"error":"Refresh token required"})");
            }

            std::string refreshToken = body["refresh_token"].get<std::string>();
            auto claims = verifyToken(refreshToken, jwtConfig, "refresh");

            if (!claims) {
                return crow::response(401, R"({"error":"Invalid or expired refresh token"})");
            }

            // Get fresh user data from Core Service
            int userId = (*claims)["user_id"].get<int>();
            auto userResult = coreClient->get("/api/core/auth/user/" + std::to_string(userId));

            if (!userResult) {
                return crow::response(404, R"({"error":"User not found"})");
            }

            // Generate new access token
            std::string newAccessToken = generateAccessToken(*userResult, jwtConfig);

            json response = {
                {"access_token", newAccessToken},
                {"token_type", "Bearer"},
                {"expires_in", jwtConfig.expirationHours * 3600}
            };
            return crow::response(200, response.dump());
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // POST /auth/logout - Logout (stateless, just acknowledge)
    CROW_ROUTE(app, "/auth/logout").methods("POST"_method)
    ([]() {
        // JWT is stateless, logout is handled client-side
        // In a production system, you might want to blacklist the token
        return crow::response(200, R"({"message":"Logged out successfully"})");
    });

    // GET /auth/me - Get current user from token (utility endpoint)
    CROW_ROUTE(app, "/auth/me").methods("GET"_method)
    ([&jwtConfig](const crow::request& req) {
        auto auth = req.get_header_value("Authorization");
        if (auth.empty() || auth.substr(0, 7) != "Bearer ") {
            return crow::response(401, R"({"error":"Authorization header required"})");
        }

        std::string token = auth.substr(7);
        auto claims = verifyToken(token, jwtConfig, "access");

        if (claims) {
            return crow::response(200, claims->dump());
        }

        return crow::response(401, R"({"error":"Invalid or expired token"})");
    });

    // Start server
    std::cout << "Auth Service starting on port " << port << std::endl;
    std::cout << "Core Service URL: " << coreUrl << std::endl;
    app.port(port).multithreaded().run();

    return 0;
}
