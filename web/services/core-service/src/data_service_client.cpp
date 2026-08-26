#include "data_service_client.h"
#include <sstream>

DataServiceClient::DataServiceClient(const std::string& dataServiceUrl)
    : client_(dataServiceUrl) {}

// ==================== USER OPERATIONS ====================

std::optional<json> DataServiceClient::getUserById(int userId) {
    return client_.get("/api/data/users/" + std::to_string(userId));
}

std::optional<json> DataServiceClient::getUserByEmail(const std::string& email) {
    return client_.get("/api/data/users/email/" + email);
}

std::vector<json> DataServiceClient::getAllUsers(int limit, int offset) {
    std::stringstream path;
    path << "/api/data/users?limit=" << limit << "&offset=" << offset;
    auto result = client_.get(path.str());
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

std::optional<json> DataServiceClient::createUser(const json& userData) {
    return client_.post("/api/data/users", userData);
}

bool DataServiceClient::updateUser(int userId, const json& userData) {
    auto result = client_.put("/api/data/users/" + std::to_string(userId), userData);
    return result.has_value();
}

bool DataServiceClient::deleteUser(int userId) {
    return client_.del("/api/data/users/" + std::to_string(userId));
}

bool DataServiceClient::updateUserRole(int userId, int role) {
    json body = {{"role", role}};
    auto result = client_.patch("/api/data/users/" + std::to_string(userId) + "/role", body);
    return result && (*result)["success"].get<bool>();
}

bool DataServiceClient::verifyCredentials(const std::string& email, const std::string& passwordHash) {
    json body = {{"email", email}, {"password_hash", passwordHash}};
    auto result = client_.post("/api/data/users/verify", body);
    return result && (*result)["valid"].get<bool>();
}

// ==================== RECIPE OPERATIONS ====================

std::vector<json> DataServiceClient::getAllRecipes(int limit, int offset) {
    std::stringstream path;
    path << "/api/data/recipes?limit=" << limit << "&offset=" << offset;
    auto result = client_.get(path.str());
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

std::optional<json> DataServiceClient::getRecipeById(int recipeId) {
    return client_.get("/api/data/recipes/" + std::to_string(recipeId));
}

std::optional<json> DataServiceClient::getRecipePreview(int recipeId) {
    return client_.get("/api/data/recipes/" + std::to_string(recipeId) + "/preview");
}

std::optional<json> DataServiceClient::getRecipeDetail(int recipeId) {
    return client_.get("/api/data/recipes/" + std::to_string(recipeId) + "/detail");
}

std::optional<json> DataServiceClient::createRecipe(const json& recipeData) {
    return client_.post("/api/data/recipes", recipeData);
}

bool DataServiceClient::updateRecipe(int recipeId, const json& recipeData) {
    auto result = client_.put("/api/data/recipes/" + std::to_string(recipeId), recipeData);
    return result.has_value();
}

bool DataServiceClient::deleteRecipe(int recipeId) {
    return client_.del("/api/data/recipes/" + std::to_string(recipeId));
}

std::vector<json> DataServiceClient::getRecipeIngredients(int recipeId) {
    auto result = client_.get("/api/data/recipes/" + std::to_string(recipeId) + "/ingredients");
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

bool DataServiceClient::addIngredientToRecipe(int recipeId, const json& ingredientData) {
    auto result = client_.post("/api/data/recipes/" + std::to_string(recipeId) + "/ingredients", ingredientData);
    return result && (*result)["success"].get<bool>();
}

bool DataServiceClient::removeIngredientFromRecipe(int recipeId, int ingredientId) {
    return client_.del("/api/data/recipes/" + std::to_string(recipeId) + "/ingredients/" + std::to_string(ingredientId));
}

std::vector<json> DataServiceClient::getRecipesByCategory(int categoryId) {
    auto result = client_.get("/api/data/recipes/by-category/" + std::to_string(categoryId));
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

std::vector<json> DataServiceClient::getRecipesByIngredients(const std::vector<int>& ingredientIds) {
    json body = {{"ingredient_ids", ingredientIds}};
    auto result = client_.post("/api/data/recipes/by-ingredients", body);
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

// ==================== CATEGORY OPERATIONS ====================

std::vector<json> DataServiceClient::getAllCategories() {
    auto result = client_.get("/api/data/categories");
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

bool DataServiceClient::addCategoryToRecipe(int recipeId, int categoryId) {
    json body = {{"category_id", categoryId}};
    auto result = client_.post("/api/data/recipes/" + std::to_string(recipeId) + "/categories", body);
    return result && (*result)["success"].get<bool>();
}

// ==================== INGREDIENT OPERATIONS ====================

std::vector<json> DataServiceClient::getAllIngredients() {
    auto result = client_.get("/api/data/ingredients");
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

std::optional<json> DataServiceClient::getIngredientById(int ingredientId) {
    return client_.get("/api/data/ingredients/" + std::to_string(ingredientId));
}

std::vector<json> DataServiceClient::getIngredientSubstitutes(int ingredientId, double threshold, int limit) {
    std::stringstream path;
    path << "/api/data/ingredients/" << ingredientId << "/substitutes?threshold=" << threshold << "&limit=" << limit;
    auto result = client_.get(path.str());
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

std::vector<json> DataServiceClient::searchIngredients(const std::string& name) {
    auto result = client_.get("/api/data/ingredients/search/" + name);
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

// ==================== FAVORITE OPERATIONS ====================

std::vector<json> DataServiceClient::getFavoritesByUserId(int userId) {
    auto result = client_.get("/api/data/favorites/user/" + std::to_string(userId));
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

bool DataServiceClient::addFavorite(int userId, int recipeId) {
    json body = {{"user_id", userId}, {"recipe_id", recipeId}};
    auto result = client_.post("/api/data/favorites", body);
    return result.has_value();
}

bool DataServiceClient::removeFavorite(int userId, int recipeId) {
    return client_.del("/api/data/favorites/" + std::to_string(userId) + "/" + std::to_string(recipeId));
}

bool DataServiceClient::isFavorite(int userId, int recipeId) {
    auto result = client_.get("/api/data/favorites/check/" + std::to_string(userId) + "/" + std::to_string(recipeId));
    return result && (*result)["is_favorite"].get<bool>();
}

// ==================== REVIEW OPERATIONS ====================

std::vector<json> DataServiceClient::getReviewsByRecipeId(int recipeId) {
    auto result = client_.get("/api/data/reviews/recipe/" + std::to_string(recipeId));
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

std::vector<json> DataServiceClient::getReviewsByUserId(int userId) {
    auto result = client_.get("/api/data/reviews/user/" + std::to_string(userId));
    if (result && result->is_array()) {
        return result->get<std::vector<json>>();
    }
    return {};
}

std::optional<json> DataServiceClient::createReview(const json& reviewData) {
    return client_.post("/api/data/reviews", reviewData);
}

bool DataServiceClient::updateReview(int reviewId, const json& reviewData) {
    auto result = client_.put("/api/data/reviews/" + std::to_string(reviewId), reviewData);
    return result.has_value();
}

bool DataServiceClient::deleteReview(int reviewId) {
    return client_.del("/api/data/reviews/" + std::to_string(reviewId));
}

double DataServiceClient::getAverageRating(int recipeId) {
    auto result = client_.get("/api/data/reviews/recipe/" + std::to_string(recipeId) + "/rating");
    if (result) {
        return (*result)["average_rating"].get<double>();
    }
    return 0.0;
}
