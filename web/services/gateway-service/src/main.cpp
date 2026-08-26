#include "crow.h"
#include "nlohmann/json.hpp"
#include "core_service_client.h"
#include "auth_service_client.h"
#include <memory>
#include <iostream>
#include <cstdlib>

using json = nlohmann::json;

// Extract token from Authorization header
std::optional<std::string> extractToken(const crow::request& req) {
    auto auth = req.get_header_value("Authorization");
    if (auth.empty() || auth.substr(0, 7) != "Bearer ") {
        return std::nullopt;
    }
    return auth.substr(7);
}

// Authenticate request via Auth Service
std::optional<json> authenticateRequest(const crow::request& req, AuthServiceClient& authClient) {
    auto token = extractToken(req);
    if (!token) return std::nullopt;
    return authClient.verifyToken(*token);
}

// Check if user has required role (0 = USER, 1 = ADMIN)
bool hasRole(const json& user, int requiredRole) {
    return user["role"].get<int>() >= requiredRole;
}

int main() {
    std::cout << "Starting Gateway Service..." << std::endl;

    // Get configuration from environment
    const char* coreServiceUrl = std::getenv("CORE_SERVICE_URL");
    const char* authServiceUrl = std::getenv("AUTH_SERVICE_URL");
    const char* portStr = std::getenv("PORT");

    std::string coreUrl = coreServiceUrl ? coreServiceUrl : "http://localhost:8081";
    std::string authUrl = authServiceUrl ? authServiceUrl : "http://localhost:8083";
    int port = portStr ? std::stoi(portStr) : 8080;

    auto coreClient = std::make_shared<CoreServiceClient>(coreUrl);
    auto authClient = std::make_shared<AuthServiceClient>(authUrl);

    crow::SimpleApp app;

    // ==================== HEALTH CHECK ====================
    CROW_ROUTE(app, "/api/v2/health")
    ([]() {
        json response = {
            {"status", "ok"},
            {"service", "gateway-service"},
            {"version", "2.0.0"}
        };
        return crow::response(200, response.dump());
    });

    // ==================== AUTH ROUTES (delegated to Auth Service) ====================

    // POST /api/v2/auth/register
    CROW_ROUTE(app, "/api/v2/auth/register").methods("POST"_method)
    ([&authClient](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            auto result = authClient->registerUser(body);

            if (result) {
                if (result->contains("error")) {
                    return crow::response(400, result->dump());
                }
                return crow::response(201, result->dump());
            }
            return crow::response(500, R"({"error":"Registration failed"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // POST /api/v2/auth/login
    CROW_ROUTE(app, "/api/v2/auth/login").methods("POST"_method)
    ([&authClient](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            std::string email = body["email"].get<std::string>();
            std::string password = body["password"].get<std::string>();

            auto result = authClient->loginUser(email, password);

            if (result) {
                if (result->contains("error")) {
                    return crow::response(401, result->dump());
                }
                return crow::response(200, result->dump());
            }

            return crow::response(401, R"({"error":"Invalid credentials"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // POST /api/v2/auth/refresh
    CROW_ROUTE(app, "/api/v2/auth/refresh").methods("POST"_method)
    ([&authClient](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            if (!body.contains("refresh_token")) {
                return crow::response(400, R"({"error":"Refresh token required"})");
            }

            std::string refreshToken = body["refresh_token"].get<std::string>();
            auto result = authClient->refreshToken(refreshToken);

            if (result) {
                if (result->contains("error")) {
                    return crow::response(401, result->dump());
                }
                return crow::response(200, result->dump());
            }

            return crow::response(401, R"({"error":"Invalid refresh token"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // POST /api/v2/auth/logout
    CROW_ROUTE(app, "/api/v2/auth/logout").methods("POST"_method)
    ([&authClient]() {
        authClient->logout();
        return crow::response(200, R"({"message":"Logged out successfully"})");
    });

    // ==================== RECIPE ROUTES ====================

    // GET /api/v2/recipes
    CROW_ROUTE(app, "/api/v2/recipes")
    ([&coreClient](const crow::request& req) {
        int limit = 100, offset = 0;
        std::string category = "", search = "";

        if (req.url_params.get("limit")) limit = std::stoi(req.url_params.get("limit"));
        if (req.url_params.get("offset")) offset = std::stoi(req.url_params.get("offset"));
        if (req.url_params.get("category")) category = req.url_params.get("category");
        if (req.url_params.get("search")) search = req.url_params.get("search");

        auto result = coreClient->getRecipes(limit, offset, category, search);
        if (result) {
            return crow::response(200, result->dump());
        }
        return crow::response(500, R"({"error":"Failed to fetch recipes"})");
    });

    // GET /api/v2/recipes/:id
    CROW_ROUTE(app, "/api/v2/recipes/<int>")
    ([&coreClient](int id) {
        auto result = coreClient->getRecipeById(id);
        if (result) {
            return crow::response(200, result->dump());
        }
        return crow::response(404, R"({"error":"Recipe not found"})");
    });

    // POST /api/v2/recipes (requires auth)
    CROW_ROUTE(app, "/api/v2/recipes").methods("POST"_method)
    ([&coreClient, &authClient](const crow::request& req) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        try {
            auto body = json::parse(req.body);
            body["author_id"] = (*user)["user_id"];

            auto result = coreClient->createRecipe(body);
            if (result) {
                return crow::response(201, result->dump());
            }
            return crow::response(500, R"({"error":"Failed to create recipe"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // PUT /api/v2/recipes/:id (requires auth)
    CROW_ROUTE(app, "/api/v2/recipes/<int>").methods("PUT"_method)
    ([&coreClient, &authClient](const crow::request& req, int id) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        try {
            auto body = json::parse(req.body);
            if (coreClient->updateRecipe(id, body)) {
                return crow::response(200, R"({"success":true})");
            }
            return crow::response(500, R"({"error":"Failed to update recipe"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // DELETE /api/v2/recipes/:id (requires auth)
    CROW_ROUTE(app, "/api/v2/recipes/<int>").methods("DELETE"_method)
    ([&coreClient, &authClient](const crow::request& req, int id) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        if (coreClient->deleteRecipe(id)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"Recipe not found"})");
    });

    // GET /api/v2/categories
    CROW_ROUTE(app, "/api/v2/categories")
    ([&coreClient]() {
        auto result = coreClient->getCategories();
        if (result) {
            return crow::response(200, result->dump());
        }
        return crow::response(500, R"({"error":"Failed to fetch categories"})");
    });

    // GET /api/v2/ingredients
    CROW_ROUTE(app, "/api/v2/ingredients")
    ([&coreClient]() {
        auto result = coreClient->getIngredients();
        if (result) {
            return crow::response(200, result->dump());
        }
        return crow::response(500, R"({"error":"Failed to fetch ingredients"})");
    });

    // GET /api/v2/ingredients/:id/substitutes
    CROW_ROUTE(app, "/api/v2/ingredients/<int>/substitutes")
    ([&coreClient](const crow::request& req, int id) {
        int limit = 3;
        if (req.url_params.get("limit")) limit = std::stoi(req.url_params.get("limit"));

        auto result = coreClient->getIngredientSubstitutes(id, limit);
        if (result) {
            return crow::response(200, result->dump());
        }
        return crow::response(404, R"({"error":"Ingredient not found"})");
    });

    // ==================== USER ROUTES ====================

    // GET /api/v2/users/me (requires auth)
    CROW_ROUTE(app, "/api/v2/users/me")
    ([&coreClient, &authClient](const crow::request& req) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        auto profile = coreClient->getUserProfile((*user)["user_id"].get<int>());
        if (profile) {
            return crow::response(200, profile->dump());
        }
        return crow::response(404, R"({"error":"User not found"})");
    });

    // PUT /api/v2/users/me (requires auth)
    CROW_ROUTE(app, "/api/v2/users/me").methods("PUT"_method)
    ([&coreClient, &authClient](const crow::request& req) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        try {
            auto body = json::parse(req.body);
            if (coreClient->updateUser((*user)["user_id"].get<int>(), body)) {
                return crow::response(200, R"({"success":true})");
            }
            return crow::response(500, R"({"error":"Failed to update user"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // DELETE /api/v2/users/me (requires auth)
    CROW_ROUTE(app, "/api/v2/users/me").methods("DELETE"_method)
    ([&coreClient, &authClient](const crow::request& req) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        if (coreClient->deleteUser((*user)["user_id"].get<int>())) {
            return crow::response(204);
        }
        return crow::response(500, R"({"error":"Failed to delete user"})");
    });

    // GET /api/v2/users/:id
    CROW_ROUTE(app, "/api/v2/users/<int>")
    ([&coreClient](int id) {
        auto profile = coreClient->getUserProfile(id);
        if (profile) {
            return crow::response(200, profile->dump());
        }
        return crow::response(404, R"({"error":"User not found"})");
    });

    // ==================== FAVORITE ROUTES ====================

    // GET /api/v2/favorites (requires auth)
    CROW_ROUTE(app, "/api/v2/favorites")
    ([&coreClient, &authClient](const crow::request& req) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        auto favorites = coreClient->getUserFavorites((*user)["user_id"].get<int>());
        if (favorites) {
            return crow::response(200, favorites->dump());
        }
        return crow::response(500, R"({"error":"Failed to fetch favorites"})");
    });

    // POST /api/v2/favorites (requires auth)
    CROW_ROUTE(app, "/api/v2/favorites").methods("POST"_method)
    ([&coreClient, &authClient](const crow::request& req) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        try {
            auto body = json::parse(req.body);
            int recipeId = body["recipe_id"].get<int>();

            if (coreClient->addFavorite((*user)["user_id"].get<int>(), recipeId)) {
                return crow::response(201, R"({"success":true})");
            }
            return crow::response(500, R"({"error":"Failed to add favorite"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // DELETE /api/v2/favorites/:recipeId (requires auth)
    CROW_ROUTE(app, "/api/v2/favorites/<int>").methods("DELETE"_method)
    ([&coreClient, &authClient](const crow::request& req, int recipeId) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        if (coreClient->removeFavorite((*user)["user_id"].get<int>(), recipeId)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"Favorite not found"})");
    });

    // ==================== REVIEW ROUTES ====================

    // GET /api/v2/recipes/:recipeId/reviews
    CROW_ROUTE(app, "/api/v2/recipes/<int>/reviews")
    ([&coreClient](int recipeId) {
        auto reviews = coreClient->getRecipeReviews(recipeId);
        if (reviews) {
            return crow::response(200, reviews->dump());
        }
        return crow::response(500, R"({"error":"Failed to fetch reviews"})");
    });

    // POST /api/v2/recipes/:recipeId/reviews (requires auth)
    CROW_ROUTE(app, "/api/v2/recipes/<int>/reviews").methods("POST"_method)
    ([&coreClient, &authClient](const crow::request& req, int recipeId) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        try {
            auto body = json::parse(req.body);
            body["user_id"] = (*user)["user_id"];

            auto result = coreClient->createReview(recipeId, body);
            if (result) {
                return crow::response(201, result->dump());
            }
            return crow::response(500, R"({"error":"Failed to create review"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // PUT /api/v2/recipes/:recipeId/reviews (requires auth)
    CROW_ROUTE(app, "/api/v2/recipes/<int>/reviews").methods("PUT"_method)
    ([&coreClient, &authClient](const crow::request& req, int recipeId) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        try {
            auto body = json::parse(req.body);
            int reviewId = body["review_id"].get<int>();
            body["user_id"] = (*user)["user_id"];

            if (coreClient->updateReview(recipeId, reviewId, body)) {
                return crow::response(200, R"({"success":true})");
            }
            return crow::response(500, R"({"error":"Failed to update review"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // DELETE /api/v2/reviews/:id (requires auth)
    CROW_ROUTE(app, "/api/v2/reviews/<int>").methods("DELETE"_method)
    ([&coreClient, &authClient](const crow::request& req, int id) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }

        if (coreClient->deleteReview(id)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"Review not found"})");
    });

    // ==================== ADMIN ROUTES ====================

    // GET /api/v2/admin/users (requires admin)
    CROW_ROUTE(app, "/api/v2/admin/users")
    ([&coreClient, &authClient](const crow::request& req) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }
        if (!hasRole(*user, 1)) {
            return crow::response(403, R"({"error":"Admin access required"})");
        }

        int limit = 100, offset = 0;
        if (req.url_params.get("limit")) limit = std::stoi(req.url_params.get("limit"));
        if (req.url_params.get("offset")) offset = std::stoi(req.url_params.get("offset"));

        auto users = coreClient->getAllUsers(limit, offset);
        if (users) {
            return crow::response(200, users->dump());
        }
        return crow::response(500, R"({"error":"Failed to fetch users"})");
    });

    // PATCH /api/v2/admin/users/:id/role (requires admin)
    CROW_ROUTE(app, "/api/v2/admin/users/<int>/role").methods("PATCH"_method)
    ([&coreClient, &authClient](const crow::request& req, int id) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }
        if (!hasRole(*user, 1)) {
            return crow::response(403, R"({"error":"Admin access required"})");
        }

        try {
            auto body = json::parse(req.body);
            int role = body["role"].get<int>();

            if (coreClient->updateUserRole(id, role)) {
                return crow::response(200, R"({"success":true})");
            }
            return crow::response(500, R"({"error":"Failed to update role"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // DELETE /api/v2/admin/users/:id (requires admin)
    CROW_ROUTE(app, "/api/v2/admin/users/<int>").methods("DELETE"_method)
    ([&coreClient, &authClient](const crow::request& req, int id) {
        auto user = authenticateRequest(req, *authClient);
        if (!user) {
            return crow::response(401, R"({"error":"Authentication required"})");
        }
        if (!hasRole(*user, 1)) {
            return crow::response(403, R"({"error":"Admin access required"})");
        }

        if (coreClient->adminDeleteUser(id)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"User not found"})");
    });

    // Start server
    std::cout << "Gateway Service starting on port " << port << std::endl;
    std::cout << "Core Service URL: " << coreUrl << std::endl;
    std::cout << "Auth Service URL: " << authUrl << std::endl;
    app.port(port).multithreaded().run();

    return 0;
}
