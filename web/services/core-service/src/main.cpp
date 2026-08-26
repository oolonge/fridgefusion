#include "crow.h"
#include "nlohmann/json.hpp"
#include "data_service_client.h"
#include <memory>
#include <iostream>
#include <cstdlib>
#include <functional>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

// Password hasher
std::string hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

int main() {
    std::cout << "Starting Core Service..." << std::endl;

    // Get configuration from environment
    const char* dataServiceUrl = std::getenv("DATA_SERVICE_URL");
    const char* portStr = std::getenv("PORT");

    std::string dataUrl = dataServiceUrl ? dataServiceUrl : "http://localhost:8082";
    int port = portStr ? std::stoi(portStr) : 8081;

    auto dataClient = std::make_shared<DataServiceClient>(dataUrl);

    crow::SimpleApp app;

    // ==================== HEALTH CHECK ====================
    CROW_ROUTE(app, "/health")
    ([]() {
        json response = {{"status", "ok"}, {"service", "core-service"}};
        return crow::response(200, response.dump());
    });

    // ==================== AUTH PROCESSOR ====================

    // POST /api/core/auth/register
    CROW_ROUTE(app, "/api/core/auth/register").methods("POST"_method)
    ([&dataClient](const crow::request& req) {
        try {
            auto body = json::parse(req.body);

            std::string username = body["username"].get<std::string>();
            std::string email = body["email"].get<std::string>();
            std::string password = body["password"].get<std::string>();

            // Check if user exists
            auto existingUser = dataClient->getUserByEmail(email);
            if (existingUser) {
                return crow::response(409, R"({"error":"User with this email already exists"})");
            }

            // Hash password
            std::string passwordHash = hashPassword(password);

            // Create user (role: 1 = USER in UserRole enum)
            json userData = {
                {"username", username},
                {"email", email},
                {"password_hash", passwordHash},
                {"role", 1}
            };

            auto newUser = dataClient->createUser(userData);
            if (newUser) {
                // Return full user data for Auth Service to generate JWT
                json response = {
                    {"id", (*newUser)["id"]},
                    {"username", (*newUser)["username"]},
                    {"email", (*newUser)["email"]},
                    {"role", (*newUser)["role"]}
                };
                return crow::response(201, response.dump());
            }
            return crow::response(500, R"({"error":"Failed to create user"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // POST /api/core/auth/login
    CROW_ROUTE(app, "/api/core/auth/login").methods("POST"_method)
    ([&dataClient](const crow::request& req) {
        try {
            auto body = json::parse(req.body);

            std::string email = body["email"].get<std::string>();
            std::string password = body["password"].get<std::string>();
            std::string passwordHash = hashPassword(password);

            // Find user
            auto user = dataClient->getUserByEmail(email);
            if (!user) {
                return crow::response(401, R"({"error":"Invalid credentials"})");
            }

            // Verify password
            if ((*user)["password_hash"].get<std::string>() != passwordHash) {
                return crow::response(401, R"({"error":"Invalid credentials"})");
            }

            // Return user info (JWT generation will be in Gateway)
            json response = {
                {"success", true},
                {"user", {
                    {"id", (*user)["id"]},
                    {"username", (*user)["username"]},
                    {"email", (*user)["email"]},
                    {"role", (*user)["role"]}
                }}
            };
            return crow::response(200, response.dump());
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // GET /api/core/auth/user/:id
    CROW_ROUTE(app, "/api/core/auth/user/<int>")
    ([&dataClient](int userId) {
        auto user = dataClient->getUserById(userId);
        if (user) {
            // Remove sensitive data
            json response = {
                {"id", (*user)["id"]},
                {"username", (*user)["username"]},
                {"email", (*user)["email"]},
                {"role", (*user)["role"]},
                {"registration_date", (*user)["registration_date"]}
            };
            return crow::response(200, response.dump());
        }
        return crow::response(404, R"({"error":"User not found"})");
    });

    // ==================== RECIPE PROCESSOR ====================

    // GET /api/core/recipes
    CROW_ROUTE(app, "/api/core/recipes")
    ([&dataClient](const crow::request& req) {
        int limit = 100, offset = 0;
        if (req.url_params.get("limit")) limit = std::stoi(req.url_params.get("limit"));
        if (req.url_params.get("offset")) offset = std::stoi(req.url_params.get("offset"));

        // Get filters
        const char* categoryParam = req.url_params.get("category");
        const char* searchParam = req.url_params.get("search");

        std::vector<json> recipes;

        if (categoryParam) {
            int categoryId = std::stoi(categoryParam);
            recipes = dataClient->getRecipesByCategory(categoryId);
        } else {
            recipes = dataClient->getAllRecipes(limit, offset);
        }

        // Get previews for each recipe
        json result = json::array();
        for (const auto& recipe : recipes) {
            auto preview = dataClient->getRecipePreview(recipe["id"].get<int>());
            if (preview) {
                // Apply search filter if present
                if (searchParam) {
                    std::string search = searchParam;
                    std::string name = (*preview)["name"].get<std::string>();
                    std::string desc = (*preview)["description"].get<std::string>();

                    // Simple case-insensitive search
                    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
                    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                    std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

                    if (name.find(search) == std::string::npos && desc.find(search) == std::string::npos) {
                        continue;
                    }
                }
                result.push_back(*preview);
            }
        }

        return crow::response(200, result.dump());
    });

    // GET /api/core/recipes/:id
    CROW_ROUTE(app, "/api/core/recipes/<int>")
    ([&dataClient](int recipeId) {
        auto detail = dataClient->getRecipeDetail(recipeId);
        if (detail) {
            return crow::response(200, detail->dump());
        }
        return crow::response(404, R"({"error":"Recipe not found"})");
    });

    // POST /api/core/recipes
    CROW_ROUTE(app, "/api/core/recipes").methods("POST"_method)
    ([&dataClient](const crow::request& req) {
        try {
            auto body = json::parse(req.body);

            // Validate required fields
            if (!body.contains("name") || !body.contains("instructions")) {
                return crow::response(400, R"({"error":"Name and instructions are required"})");
            }

            json recipeData = {
                {"name", body["name"]},
                {"description", body.value("description", "")},
                {"preparation_time", body.value("preparation_time", 0)},
                {"cooking_time", body.value("cooking_time", 0)},
                {"instructions", body["instructions"]},
                {"author_id", body.value("author_id", 0)}
            };

            auto result = dataClient->createRecipe(recipeData);
            if (result) {
                int recipeId = (*result)["id"].get<int>();

                // Add categories if provided
                if (body.contains("category_ids")) {
                    for (auto& catId : body["category_ids"]) {
                        dataClient->addCategoryToRecipe(recipeId, catId.get<int>());
                    }
                }

                // Add ingredients if provided
                if (body.contains("ingredients")) {
                    for (auto& ing : body["ingredients"]) {
                        json ingData = {
                            {"ingredient_id", ing["ingredient_id"]},
                            {"quantity", ing["quantity"]},
                            {"unit", ing.value("unit", "")},
                            {"is_optional", ing.value("is_optional", false)}
                        };
                        dataClient->addIngredientToRecipe(recipeId, ingData);
                    }
                }

                json response = {{"id", recipeId}, {"success", true}};
                return crow::response(201, response.dump());
            }
            return crow::response(500, R"({"error":"Failed to create recipe"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // PUT /api/core/recipes/:id
    CROW_ROUTE(app, "/api/core/recipes/<int>").methods("PUT"_method)
    ([&dataClient](const crow::request& req, int recipeId) {
        try {
            auto body = json::parse(req.body);

            json recipeData = {
                {"name", body["name"]},
                {"description", body.value("description", "")},
                {"preparation_time", body.value("preparation_time", 0)},
                {"cooking_time", body.value("cooking_time", 0)},
                {"instructions", body["instructions"]}
            };

            if (dataClient->updateRecipe(recipeId, recipeData)) {
                return crow::response(200, R"({"success":true})");
            }
            return crow::response(500, R"({"error":"Failed to update recipe"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // DELETE /api/core/recipes/:id
    CROW_ROUTE(app, "/api/core/recipes/<int>").methods("DELETE"_method)
    ([&dataClient](int recipeId) {
        if (dataClient->deleteRecipe(recipeId)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"Recipe not found"})");
    });

    // GET /api/core/categories
    CROW_ROUTE(app, "/api/core/categories")
    ([&dataClient]() {
        auto categories = dataClient->getAllCategories();
        json result = json::array();
        for (const auto& c : categories) {
            result.push_back(c);
        }
        return crow::response(200, result.dump());
    });

    // GET /api/core/ingredients
    CROW_ROUTE(app, "/api/core/ingredients")
    ([&dataClient]() {
        auto ingredients = dataClient->getAllIngredients();
        json result = json::array();
        for (const auto& i : ingredients) {
            result.push_back(i);
        }
        return crow::response(200, result.dump());
    });

    // GET /api/core/ingredients/:id/substitutes
    CROW_ROUTE(app, "/api/core/ingredients/<int>/substitutes")
    ([&dataClient](const crow::request& req, int ingredientId) {
        int limit = 3;
        if (req.url_params.get("limit")) limit = std::stoi(req.url_params.get("limit"));

        auto substitutes = dataClient->getIngredientSubstitutes(ingredientId, 0.7, limit);

        // Enrich with ingredient details
        json result = json::array();
        for (const auto& sub : substitutes) {
            auto ingredient = dataClient->getIngredientById(sub["ingredient_id"].get<int>());
            if (ingredient) {
                json item = *ingredient;
                item["similarity"] = sub["similarity"];
                result.push_back(item);
            }
        }
        return crow::response(200, result.dump());
    });

    // ==================== USER PROCESSOR ====================

    // GET /api/core/users/:id/profile
    CROW_ROUTE(app, "/api/core/users/<int>/profile")
    ([&dataClient](int userId) {
        auto user = dataClient->getUserById(userId);
        if (!user) {
            return crow::response(404, R"({"error":"User not found"})");
        }

        // Get user's favorites count
        auto favorites = dataClient->getFavoritesByUserId(userId);

        // Get user's reviews count
        auto reviews = dataClient->getReviewsByUserId(userId);

        json profile = {
            {"id", (*user)["id"]},
            {"username", (*user)["username"]},
            {"email", (*user)["email"]},
            {"registration_date", (*user)["registration_date"]},
            {"favorites_count", favorites.size()},
            {"reviews_count", reviews.size()}
        };

        return crow::response(200, profile.dump());
    });

    // PUT /api/core/users/:id
    CROW_ROUTE(app, "/api/core/users/<int>").methods("PUT"_method)
    ([&dataClient](const crow::request& req, int userId) {
        try {
            auto body = json::parse(req.body);

            json userData;
            if (body.contains("username")) userData["username"] = body["username"];
            if (body.contains("email")) userData["email"] = body["email"];
            if (body.contains("password")) {
                userData["password_hash"] = hashPassword(body["password"].get<std::string>());
            }

            if (dataClient->updateUser(userId, userData)) {
                return crow::response(200, R"({"success":true})");
            }
            return crow::response(500, R"({"error":"Failed to update user"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // DELETE /api/core/users/:id
    CROW_ROUTE(app, "/api/core/users/<int>").methods("DELETE"_method)
    ([&dataClient](int userId) {
        if (dataClient->deleteUser(userId)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"User not found"})");
    });

    // ==================== FAVORITE PROCESSOR ====================

    // GET /api/core/users/:userId/favorites
    CROW_ROUTE(app, "/api/core/users/<int>/favorites")
    ([&dataClient](int userId) {
        auto favorites = dataClient->getFavoritesByUserId(userId);

        json result = json::array();
        for (const auto& fav : favorites) {
            auto recipe = dataClient->getRecipePreview(fav["recipe_id"].get<int>());
            if (recipe) {
                json item = *recipe;
                item["date_added"] = fav["date_added"];
                result.push_back(item);
            }
        }
        return crow::response(200, result.dump());
    });

    // POST /api/core/users/:userId/favorites
    CROW_ROUTE(app, "/api/core/users/<int>/favorites").methods("POST"_method)
    ([&dataClient](const crow::request& req, int userId) {
        try {
            auto body = json::parse(req.body);
            int recipeId = body["recipe_id"].get<int>();

            // Check if recipe exists
            auto recipe = dataClient->getRecipeById(recipeId);
            if (!recipe) {
                return crow::response(404, R"({"error":"Recipe not found"})");
            }

            // Check if already favorite
            if (dataClient->isFavorite(userId, recipeId)) {
                return crow::response(409, R"({"error":"Already in favorites"})");
            }

            if (dataClient->addFavorite(userId, recipeId)) {
                return crow::response(201, R"({"success":true})");
            }
            return crow::response(500, R"({"error":"Failed to add favorite"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // DELETE /api/core/users/:userId/favorites/:recipeId
    CROW_ROUTE(app, "/api/core/users/<int>/favorites/<int>").methods("DELETE"_method)
    ([&dataClient](int userId, int recipeId) {
        if (dataClient->removeFavorite(userId, recipeId)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"Favorite not found"})");
    });

    // ==================== REVIEW PROCESSOR ====================

    // GET /api/core/recipes/:recipeId/reviews
    CROW_ROUTE(app, "/api/core/recipes/<int>/reviews")
    ([&dataClient](int recipeId) {
        auto reviews = dataClient->getReviewsByRecipeId(recipeId);

        json result = json::array();
        for (const auto& rev : reviews) {
            // Get user info
            auto user = dataClient->getUserById(rev["user_id"].get<int>());
            json item = rev;
            if (user) {
                item["username"] = (*user)["username"];
            }
            result.push_back(item);
        }
        return crow::response(200, result.dump());
    });

    // POST /api/core/recipes/:recipeId/reviews
    CROW_ROUTE(app, "/api/core/recipes/<int>/reviews").methods("POST"_method)
    ([&dataClient](const crow::request& req, int recipeId) {
        try {
            auto body = json::parse(req.body);

            // Validate rating
            int rating = body["rating"].get<int>();
            if (rating < 1 || rating > 5) {
                return crow::response(400, R"({"error":"Rating must be between 1 and 5"})");
            }

            json reviewData = {
                {"user_id", body["user_id"]},
                {"recipe_id", recipeId},
                {"rating", rating},
                {"comment", body.value("comment", "")}
            };

            auto result = dataClient->createReview(reviewData);
            if (result) {
                return crow::response(201, result->dump());
            }
            return crow::response(500, R"({"error":"Failed to create review"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // PUT /api/core/recipes/:recipeId/reviews/:reviewId
    CROW_ROUTE(app, "/api/core/recipes/<int>/reviews/<int>").methods("PUT"_method)
    ([&dataClient](const crow::request& req, int recipeId, int reviewId) {
        try {
            auto body = json::parse(req.body);

            json reviewData = {
                {"user_id", body["user_id"]},
                {"recipe_id", recipeId},
                {"rating", body["rating"]},
                {"comment", body.value("comment", "")},
                {"date_posted", body["date_posted"]}
            };

            if (dataClient->updateReview(reviewId, reviewData)) {
                return crow::response(200, R"({"success":true})");
            }
            return crow::response(500, R"({"error":"Failed to update review"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // DELETE /api/core/reviews/:reviewId
    CROW_ROUTE(app, "/api/core/reviews/<int>").methods("DELETE"_method)
    ([&dataClient](int reviewId) {
        if (dataClient->deleteReview(reviewId)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"Review not found"})");
    });

    // ==================== ADMIN PROCESSOR ====================

    // GET /api/core/admin/users
    CROW_ROUTE(app, "/api/core/admin/users")
    ([&dataClient](const crow::request& req) {
        int limit = 100, offset = 0;
        if (req.url_params.get("limit")) limit = std::stoi(req.url_params.get("limit"));
        if (req.url_params.get("offset")) offset = std::stoi(req.url_params.get("offset"));

        auto users = dataClient->getAllUsers(limit, offset);

        json result = json::array();
        for (const auto& u : users) {
            json user = {
                {"id", u["id"]},
                {"username", u["username"]},
                {"email", u["email"]},
                {"role", u["role"]},
                {"registration_date", u["registration_date"]}
            };
            result.push_back(user);
        }
        return crow::response(200, result.dump());
    });

    // PATCH /api/core/admin/users/:id/role
    CROW_ROUTE(app, "/api/core/admin/users/<int>/role").methods("PATCH"_method)
    ([&dataClient](const crow::request& req, int userId) {
        try {
            auto body = json::parse(req.body);
            int role = body["role"].get<int>();

            if (dataClient->updateUserRole(userId, role)) {
                return crow::response(200, R"({"success":true})");
            }
            return crow::response(500, R"({"error":"Failed to update role"})");
        } catch (const std::exception& e) {
            json error = {{"error", e.what()}};
            return crow::response(400, error.dump());
        }
    });

    // DELETE /api/core/admin/users/:id
    CROW_ROUTE(app, "/api/core/admin/users/<int>").methods("DELETE"_method)
    ([&dataClient](int userId) {
        if (dataClient->deleteUser(userId)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"User not found"})");
    });

    // Start server
    std::cout << "Core Service starting on port " << port << std::endl;
    std::cout << "Data Service URL: " << dataUrl << std::endl;
    app.port(port).multithreaded().run();

    return 0;
}
