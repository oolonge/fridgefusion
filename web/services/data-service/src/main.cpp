#include "crow.h"
#include "nlohmann/json.hpp"
#include "infrastructure/persistence/database/dbContext.h"
#include "infrastructure/persistence/postgresql/userRepository.h"
#include "infrastructure/persistence/postgresql/recipeRepository.h"
#include "infrastructure/persistence/postgresql/ingredientRepository.h"
#include "infrastructure/persistence/postgresql/favoriteRepository.h"
#include "infrastructure/persistence/postgresql/reviewRepository.h"
#include "infrastructure/config/appConfig.h"
#include <memory>
#include <iostream>
#include <locale>
#include <clocale>
#include <cstdlib>

using json = nlohmann::json;

// Helper to convert QString to std::string
inline std::string qstr(const QString& s) { return s.toStdString(); }

// Helper to convert QDateTime to ISO string
inline std::string dateStr(const QDateTime& dt) {
    return dt.isValid() ? dt.toString(Qt::ISODate).toStdString() : "";
}

// Convert User to JSON
json userToJson(const User& user) {
    return {
        {"id", user.getId()},
        {"username", qstr(user.getUsername())},
        {"email", qstr(user.getEmail())},
        {"password_hash", qstr(user.getPasswordHash())},
        {"registration_date", dateStr(user.getRegistrationDate())},
        {"role", static_cast<int>(user.getRole())}
    };
}

// Convert Recipe to JSON
json recipeToJson(const Recipe& recipe) {
    json categoriesArr = json::array();
    for (int catId : recipe.getCategoryIds()) {
        categoriesArr.push_back(catId);
    }

    json stepsArr = json::array();
    for (const QString& step : recipe.getSteps()) {
        stepsArr.push_back(qstr(step));
    }

    return {
        {"id", recipe.getRecipeId()},
        {"name", qstr(recipe.getName())},
        {"description", qstr(recipe.getDescription())},
        {"preparation_time", recipe.getPreparationTime()},
        {"cooking_time", recipe.getCookingTime()},
        {"instructions", stepsArr},
        {"category_ids", categoriesArr}
    };
}

// Convert Ingredient to JSON
json ingredientToJson(const Ingredient& ing) {
    return {
        {"id", ing.getIngredientId()},
        {"name", qstr(ing.getName())},
        {"default_unit", qstr(ing.getDefaultUnit())},
        {"calories_per_100g", ing.getCalories()}
    };
}

// Convert Category to JSON
json categoryToJson(const Category& cat) {
    return {
        {"id", cat.getCategoryId()},
        {"name", qstr(cat.getName())},
        {"description", qstr(cat.getDescription())}
    };
}

// Convert Favorite to JSON
json favoriteToJson(const Favorite& fav) {
    return {
        {"id", fav.getId()},
        {"user_id", fav.getUserId()},
        {"recipe_id", fav.getRecipeId()},
        {"date_added", dateStr(fav.getDateAdded())}
    };
}

// Convert Review to JSON
json reviewToJson(const Review& rev) {
    return {
        {"id", rev.getId()},
        {"user_id", rev.getUserId()},
        {"recipe_id", rev.getRecipeId()},
        {"rating", rev.getRating()},
        {"comment", qstr(rev.getComment())},
        {"date_posted", dateStr(rev.getDatePosted())},
        {"username", qstr(rev.getUsername())}
    };
}

// Convert RecipeIngredient to JSON
json recipeIngredientToJson(const RecipeIngredient& ri) {
    return {
        {"recipe_id", ri.getRecipeId()},
        {"ingredient_id", ri.getIngredientId()},
        {"quantity", ri.getQuantity()},
        {"unit", qstr(ri.getUnit())},
        {"is_optional", ri.isOptional()}
    };
}

// Convert RecipePreviewDTO to JSON
json recipePreviewToJson(const RecipePreviewDTO& dto) {
    json mainIngredientsArr = json::array();
    for (const QString& ing : dto.mainIngredients) {
        mainIngredientsArr.push_back(qstr(ing));
    }

    return {
        {"id", dto.id},
        {"name", qstr(dto.name)},
        {"description", qstr(dto.shortDescription)},
        {"total_time", dto.totalTime},
        {"main_ingredients", mainIngredientsArr},
        {"average_rating", dto.averageRating},
        {"is_favorite", dto.isFavorite},
        {"author_name", qstr(dto.authorName)}
    };
}

// Convert RecipeDetailDTO to JSON
json recipeDetailToJson(const RecipeDetailDTO& dto) {
    json categoriesArr = json::array();
    for (const QString& cat : dto.categories) {
        categoriesArr.push_back(qstr(cat));
    }

    json ingredientsArr = json::array();
    for (const auto& ing : dto.ingredients) {
        ingredientsArr.push_back({
            {"id", ing.id},
            {"name", qstr(ing.name)},
            {"quantity", ing.quantity},
            {"unit", qstr(ing.unit)},
            {"is_optional", ing.isOptional}
        });
    }

    json stepsArr = json::array();
    for (const QString& step : dto.steps) {
        stepsArr.push_back(qstr(step));
    }

    return {
        {"id", dto.id},
        {"name", qstr(dto.name)},
        {"description", qstr(dto.fullDescription)},
        {"preparation_time", dto.preparationTime},
        {"cooking_time", dto.cookingTime},
        {"instructions", stepsArr},
        {"categories", categoriesArr},
        {"ingredients", ingredientsArr},
        {"average_rating", dto.averageRating},
        {"review_count", dto.reviewCount},
        {"is_favorite", dto.isFavorite}
    };
}

int main() {
    std::setlocale(LC_NUMERIC, "C");
    std::locale::global(std::locale::classic());

    std::cout << "Starting Data Service..." << std::endl;

    // Load config (for database connection)
    AppConfig& config = AppConfig::getInstance();

    // Get port from environment variable or default to 8082
    const char* portStr = std::getenv("PORT");
    int port = portStr ? std::stoi(portStr) : 8082;

    // Initialize infrastructure
    auto dbContext = std::make_shared<DbContext>();
    auto userRepo = std::make_shared<UserRepository>(dbContext);
    auto recipeRepo = std::make_shared<RecipeRepository>(dbContext);
    auto ingredientRepo = std::make_shared<IngredientRepository>(dbContext);
    auto favoriteRepo = std::make_shared<FavoriteRepository>(dbContext.get());
    auto reviewRepo = std::make_shared<ReviewRepository>(dbContext.get());

    crow::SimpleApp app;

    // ==================== HEALTH CHECK ====================
    CROW_ROUTE(app, "/health")
    ([]() {
        json response = {{"status", "ok"}, {"service", "data-service"}};
        return crow::response(200, response.dump());
    });

    // ==================== USER REPOSITORY ====================

    // GET /api/data/users/:id
    CROW_ROUTE(app, "/api/data/users/<int>")
    ([&userRepo](int id) {
        auto user = userRepo->getUserById(id);
        if (user) {
            return crow::response(200, userToJson(*user).dump());
        }
        return crow::response(404, R"({"error":"User not found"})");
    });

    // GET /api/data/users/email/:email
    CROW_ROUTE(app, "/api/data/users/email/<string>")
    ([&userRepo](const std::string& email) {
        auto user = userRepo->getUserByEmail(QString::fromStdString(email));
        if (user) {
            return crow::response(200, userToJson(*user).dump());
        }
        return crow::response(404, R"({"error":"User not found"})");
    });

    // GET /api/data/users
    CROW_ROUTE(app, "/api/data/users")
    ([&userRepo](const crow::request& req) {
        int limit = 100, offset = 0;
        if (req.url_params.get("limit")) limit = std::stoi(req.url_params.get("limit"));
        if (req.url_params.get("offset")) offset = std::stoi(req.url_params.get("offset"));

        auto users = userRepo->getAllUsers(limit, offset);
        json arr = json::array();
        for (const auto& u : users) {
            arr.push_back(userToJson(u));
        }
        return crow::response(200, arr.dump());
    });

    // POST /api/data/users
    CROW_ROUTE(app, "/api/data/users").methods("POST"_method)
    ([&userRepo](const crow::request& req) {
        auto body = json::parse(req.body);
        User user;
        user.setUsername(QString::fromStdString(body["username"].get<std::string>()));
        user.setEmail(QString::fromStdString(body["email"].get<std::string>()));
        user.setPasswordHash(QString::fromStdString(body["password_hash"].get<std::string>()));
        user.setRole(static_cast<UserRole>(body.value("role", 0)));

        if (userRepo->saveUser(user)) {
            return crow::response(201, userToJson(user).dump());
        }
        return crow::response(500, R"({"error":"Failed to create user"})");
    });

    // PUT /api/data/users/:id
    CROW_ROUTE(app, "/api/data/users/<int>").methods("PUT"_method)
    ([&userRepo](const crow::request& req, int id) {
        auto body = json::parse(req.body);
        auto existingUser = userRepo->getUserById(id);
        if (!existingUser) {
            return crow::response(404, R"({"error":"User not found"})");
        }

        User user = *existingUser;
        if (body.contains("username")) user.setUsername(QString::fromStdString(body["username"].get<std::string>()));
        if (body.contains("email")) user.setEmail(QString::fromStdString(body["email"].get<std::string>()));
        if (body.contains("password_hash")) user.setPasswordHash(QString::fromStdString(body["password_hash"].get<std::string>()));

        if (userRepo->updateUser(user)) {
            return crow::response(200, userToJson(user).dump());
        }
        return crow::response(500, R"({"error":"Failed to update user"})");
    });

    // DELETE /api/data/users/:id
    CROW_ROUTE(app, "/api/data/users/<int>").methods("DELETE"_method)
    ([&userRepo](int id) {
        if (userRepo->deleteUser(id)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"User not found"})");
    });

    // PATCH /api/data/users/:id/role
    CROW_ROUTE(app, "/api/data/users/<int>/role").methods("PATCH"_method)
    ([&userRepo](const crow::request& req, int id) {
        auto body = json::parse(req.body);
        UserRole role = static_cast<UserRole>(body["role"].get<int>());

        if (userRepo->updateUserRole(id, role)) {
            return crow::response(200, R"({"success":true})");
        }
        return crow::response(500, R"({"error":"Failed to update role"})");
    });

    // POST /api/data/users/verify
    CROW_ROUTE(app, "/api/data/users/verify").methods("POST"_method)
    ([&userRepo](const crow::request& req) {
        auto body = json::parse(req.body);
        QString email = QString::fromStdString(body["email"].get<std::string>());
        QString passwordHash = QString::fromStdString(body["password_hash"].get<std::string>());

        bool valid = userRepo->verifyCredentials(email, passwordHash);
        json response = {{"valid", valid}};
        return crow::response(200, response.dump());
    });

    // ==================== RECIPE REPOSITORY ====================

    // GET /api/data/recipes
    CROW_ROUTE(app, "/api/data/recipes")
    ([&recipeRepo](const crow::request& req) {
        int limit = 100, offset = 0;
        if (req.url_params.get("limit")) limit = std::stoi(req.url_params.get("limit"));
        if (req.url_params.get("offset")) offset = std::stoi(req.url_params.get("offset"));

        auto recipes = recipeRepo->getAllRecipes(limit, offset);
        json arr = json::array();
        for (const auto& r : recipes) {
            arr.push_back(recipeToJson(r));
        }
        return crow::response(200, arr.dump());
    });

    // GET /api/data/recipes/:id
    CROW_ROUTE(app, "/api/data/recipes/<int>")
    ([&recipeRepo](int id) {
        auto recipe = recipeRepo->getRecipeById(id);
        if (recipe) {
            return crow::response(200, recipeToJson(*recipe).dump());
        }
        return crow::response(404, R"({"error":"Recipe not found"})");
    });

    // GET /api/data/recipes/:id/preview
    CROW_ROUTE(app, "/api/data/recipes/<int>/preview")
    ([&recipeRepo](int id) {
        auto preview = recipeRepo->getRecipePreview(id);
        if (preview.id > 0) {
            return crow::response(200, recipePreviewToJson(preview).dump());
        }
        return crow::response(404, R"({"error":"Recipe not found"})");
    });

    // GET /api/data/recipes/:id/detail
    CROW_ROUTE(app, "/api/data/recipes/<int>/detail")
    ([&recipeRepo](int id) {
        auto detail = recipeRepo->getRecipeDetail(id);
        if (detail.id > 0) {
            return crow::response(200, recipeDetailToJson(detail).dump());
        }
        return crow::response(404, R"({"error":"Recipe not found"})");
    });

    // POST /api/data/recipes
    CROW_ROUTE(app, "/api/data/recipes").methods("POST"_method)
    ([&recipeRepo](const crow::request& req) {
        auto body = json::parse(req.body);

        int recipeId = recipeRepo->createRecipe(
            QString::fromStdString(body["name"].get<std::string>()),
            QString::fromStdString(body["description"].get<std::string>()),
            body["preparation_time"].get<int>(),
            body["cooking_time"].get<int>(),
            QString::fromStdString(body["instructions"].get<std::string>()),
            body["author_id"].get<int>()
        );

        if (recipeId > 0) {
            json response = {{"id", recipeId}};
            return crow::response(201, response.dump());
        }
        return crow::response(500, R"({"error":"Failed to create recipe"})");
    });

    // PUT /api/data/recipes/:id
    CROW_ROUTE(app, "/api/data/recipes/<int>").methods("PUT"_method)
    ([&recipeRepo](const crow::request& req, int id) {
        auto body = json::parse(req.body);

        bool success = recipeRepo->updateRecipe(
            id,
            QString::fromStdString(body["name"].get<std::string>()),
            QString::fromStdString(body["description"].get<std::string>()),
            body["preparation_time"].get<int>(),
            body["cooking_time"].get<int>(),
            QString::fromStdString(body["instructions"].get<std::string>())
        );

        if (success) {
            return crow::response(200, R"({"success":true})");
        }
        return crow::response(500, R"({"error":"Failed to update recipe"})");
    });

    // DELETE /api/data/recipes/:id
    CROW_ROUTE(app, "/api/data/recipes/<int>").methods("DELETE"_method)
    ([&recipeRepo](int id) {
        if (recipeRepo->deleteRecipe(id)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"Recipe not found"})");
    });

    // GET /api/data/recipes/:id/ingredients
    CROW_ROUTE(app, "/api/data/recipes/<int>/ingredients")
    ([&recipeRepo](int id) {
        auto ingredients = recipeRepo->getRecipeIngredients(id);
        json arr = json::array();
        for (const auto& ri : ingredients) {
            arr.push_back(recipeIngredientToJson(ri));
        }
        return crow::response(200, arr.dump());
    });

    // POST /api/data/recipes/:id/ingredients
    CROW_ROUTE(app, "/api/data/recipes/<int>/ingredients").methods("POST"_method)
    ([&recipeRepo](const crow::request& req, int recipeId) {
        auto body = json::parse(req.body);

        bool success = recipeRepo->addIngredientToRecipe(
            recipeId,
            body["ingredient_id"].get<int>(),
            body["quantity"].get<double>(),
            QString::fromStdString(body["unit"].get<std::string>()),
            body.value("is_optional", false)
        );

        if (success) {
            return crow::response(201, R"({"success":true})");
        }
        return crow::response(500, R"({"error":"Failed to add ingredient"})");
    });

    // DELETE /api/data/recipes/:recipeId/ingredients/:ingredientId
    CROW_ROUTE(app, "/api/data/recipes/<int>/ingredients/<int>").methods("DELETE"_method)
    ([&recipeRepo](int recipeId, int ingredientId) {
        if (recipeRepo->removeIngredientFromRecipe(recipeId, ingredientId)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"Ingredient not found in recipe"})");
    });

    // GET /api/data/categories
    CROW_ROUTE(app, "/api/data/categories")
    ([&recipeRepo]() {
        auto categories = recipeRepo->getAllCategories();
        json arr = json::array();
        for (const auto& c : categories) {
            arr.push_back(categoryToJson(c));
        }
        return crow::response(200, arr.dump());
    });

    // POST /api/data/recipes/:id/categories
    CROW_ROUTE(app, "/api/data/recipes/<int>/categories").methods("POST"_method)
    ([&recipeRepo](const crow::request& req, int recipeId) {
        auto body = json::parse(req.body);
        int categoryId = body["category_id"].get<int>();

        if (recipeRepo->addCategoryToRecipe(recipeId, categoryId)) {
            return crow::response(201, R"({"success":true})");
        }
        return crow::response(500, R"({"error":"Failed to add category"})");
    });

    // GET /api/data/recipes/by-category/:categoryId
    CROW_ROUTE(app, "/api/data/recipes/by-category/<int>")
    ([&recipeRepo](int categoryId) {
        auto recipes = recipeRepo->getRecipesByCategory(categoryId);
        json arr = json::array();
        for (const auto& r : recipes) {
            arr.push_back(recipeToJson(r));
        }
        return crow::response(200, arr.dump());
    });

    // POST /api/data/recipes/by-ingredients
    CROW_ROUTE(app, "/api/data/recipes/by-ingredients").methods("POST"_method)
    ([&recipeRepo](const crow::request& req) {
        auto body = json::parse(req.body);
        QList<int> ingredientIds;
        for (auto& id : body["ingredient_ids"]) {
            ingredientIds.append(id.get<int>());
        }

        auto recipes = recipeRepo->getRecipesByIngredients(ingredientIds);
        json arr = json::array();
        for (const auto& r : recipes) {
            arr.push_back(recipeToJson(r));
        }
        return crow::response(200, arr.dump());
    });

    // ==================== INGREDIENT REPOSITORY ====================

    // GET /api/data/ingredients
    CROW_ROUTE(app, "/api/data/ingredients")
    ([&ingredientRepo]() {
        auto ingredients = ingredientRepo->getAllIngredients();
        json arr = json::array();
        for (const auto& i : ingredients) {
            arr.push_back(ingredientToJson(i));
        }
        return crow::response(200, arr.dump());
    });

    // GET /api/data/ingredients/:id
    CROW_ROUTE(app, "/api/data/ingredients/<int>")
    ([&ingredientRepo](int id) {
        auto ingredient = ingredientRepo->getIngredientById(id);
        if (ingredient) {
            return crow::response(200, ingredientToJson(*ingredient).dump());
        }
        return crow::response(404, R"({"error":"Ingredient not found"})");
    });

    // GET /api/data/ingredients/:id/substitutes
    CROW_ROUTE(app, "/api/data/ingredients/<int>/substitutes")
    ([&ingredientRepo](const crow::request& req, int id) {
        double threshold = 0.7;
        int limit = 3;
        if (req.url_params.get("threshold")) threshold = std::stod(req.url_params.get("threshold"));
        if (req.url_params.get("limit")) limit = std::stoi(req.url_params.get("limit"));

        auto substitutes = ingredientRepo->getIngredientSubstitutes(id, threshold, limit);
        json arr = json::array();
        for (const auto& [subId, similarity] : substitutes) {
            arr.push_back({{"ingredient_id", subId}, {"similarity", similarity}});
        }
        return crow::response(200, arr.dump());
    });

    // GET /api/data/ingredients/search/:name
    CROW_ROUTE(app, "/api/data/ingredients/search/<string>")
    ([&ingredientRepo](const std::string& name) {
        auto ingredients = ingredientRepo->getIngredientsByName(QString::fromStdString(name));
        json arr = json::array();
        for (const auto& i : ingredients) {
            arr.push_back(ingredientToJson(i));
        }
        return crow::response(200, arr.dump());
    });

    // ==================== FAVORITE REPOSITORY ====================

    // GET /api/data/favorites/user/:userId
    CROW_ROUTE(app, "/api/data/favorites/user/<int>")
    ([&favoriteRepo](int userId) {
        auto favorites = favoriteRepo->getFavoritesByUserId(userId);
        json arr = json::array();
        for (const auto& f : favorites) {
            arr.push_back(favoriteToJson(f));
        }
        return crow::response(200, arr.dump());
    });

    // POST /api/data/favorites
    CROW_ROUTE(app, "/api/data/favorites").methods("POST"_method)
    ([&favoriteRepo](const crow::request& req) {
        auto body = json::parse(req.body);
        Favorite fav(
            0,  // id will be assigned by DB
            body["user_id"].get<int>(),
            body["recipe_id"].get<int>(),
            QDateTime::currentDateTime()
        );

        if (favoriteRepo->addFavorite(fav)) {
            return crow::response(201, favoriteToJson(fav).dump());
        }
        return crow::response(500, R"({"error":"Failed to add favorite"})");
    });

    // DELETE /api/data/favorites/:userId/:recipeId
    CROW_ROUTE(app, "/api/data/favorites/<int>/<int>").methods("DELETE"_method)
    ([&favoriteRepo](int userId, int recipeId) {
        if (favoriteRepo->removeFavorite(userId, recipeId)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"Favorite not found"})");
    });

    // GET /api/data/favorites/check/:userId/:recipeId
    CROW_ROUTE(app, "/api/data/favorites/check/<int>/<int>")
    ([&favoriteRepo](int userId, int recipeId) {
        bool isFav = favoriteRepo->isFavorite(userId, recipeId);
        json response = {{"is_favorite", isFav}};
        return crow::response(200, response.dump());
    });

    // ==================== REVIEW REPOSITORY ====================

    // GET /api/data/reviews/recipe/:recipeId
    CROW_ROUTE(app, "/api/data/reviews/recipe/<int>")
    ([&reviewRepo](int recipeId) {
        auto reviews = reviewRepo->getReviewsByRecipeId(recipeId);
        json arr = json::array();
        for (const auto& r : reviews) {
            arr.push_back(reviewToJson(r));
        }
        return crow::response(200, arr.dump());
    });

    // GET /api/data/reviews/user/:userId
    CROW_ROUTE(app, "/api/data/reviews/user/<int>")
    ([&reviewRepo](int userId) {
        auto reviews = reviewRepo->getReviewsByUserId(userId);
        json arr = json::array();
        for (const auto& r : reviews) {
            arr.push_back(reviewToJson(r));
        }
        return crow::response(200, arr.dump());
    });

    // POST /api/data/reviews
    CROW_ROUTE(app, "/api/data/reviews").methods("POST"_method)
    ([&reviewRepo](const crow::request& req) {
        auto body = json::parse(req.body);
        Review review(
            0,  // id will be assigned by DB
            body["user_id"].get<int>(),
            body["recipe_id"].get<int>(),
            body["rating"].get<int>(),
            QString::fromStdString(body["comment"].get<std::string>()),
            QDateTime::currentDateTime()
        );

        if (reviewRepo->addReview(review)) {
            return crow::response(201, reviewToJson(review).dump());
        }
        return crow::response(500, R"({"error":"Failed to add review"})");
    });

    // PUT /api/data/reviews/:id
    CROW_ROUTE(app, "/api/data/reviews/<int>").methods("PUT"_method)
    ([&reviewRepo](const crow::request& req, int id) {
        auto body = json::parse(req.body);
        Review review(
            id,
            body["user_id"].get<int>(),
            body["recipe_id"].get<int>(),
            body["rating"].get<int>(),
            QString::fromStdString(body["comment"].get<std::string>()),
            QDateTime::fromString(QString::fromStdString(body["date_posted"].get<std::string>()), Qt::ISODate)
        );

        if (reviewRepo->updateReview(review)) {
            return crow::response(200, reviewToJson(review).dump());
        }
        return crow::response(500, R"({"error":"Failed to update review"})");
    });

    // DELETE /api/data/reviews/:id
    CROW_ROUTE(app, "/api/data/reviews/<int>").methods("DELETE"_method)
    ([&reviewRepo](int id) {
        if (reviewRepo->deleteReview(id)) {
            return crow::response(204);
        }
        return crow::response(404, R"({"error":"Review not found"})");
    });

    // GET /api/data/reviews/recipe/:recipeId/rating
    CROW_ROUTE(app, "/api/data/reviews/recipe/<int>/rating")
    ([&reviewRepo](int recipeId) {
        double rating = reviewRepo->getAverageRatingForRecipe(recipeId);
        json response = {{"average_rating", rating}};
        return crow::response(200, response.dump());
    });

    // Start server
    std::cout << "Data Service starting on port " << port << std::endl;
    app.port(port).multithreaded().run();

    return 0;
}
