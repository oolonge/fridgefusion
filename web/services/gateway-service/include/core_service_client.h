#ifndef CORE_SERVICE_CLIENT_H
#define CORE_SERVICE_CLIENT_H

#include <string>
#include <optional>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class CoreServiceClient {
public:
    CoreServiceClient(const std::string& coreServiceUrl);

    // Auth operations
    std::optional<json> registerUser(const json& userData);
    std::optional<json> loginUser(const std::string& email, const std::string& password);
    std::optional<json> getUserById(int userId);

    // Recipe operations
    std::optional<json> getRecipes(int limit, int offset, const std::string& category = "", const std::string& search = "");
    std::optional<json> getRecipeById(int recipeId);
    std::optional<json> createRecipe(const json& recipeData);
    bool updateRecipe(int recipeId, const json& recipeData);
    bool deleteRecipe(int recipeId);

    // Category & Ingredient operations
    std::optional<json> getCategories();
    std::optional<json> getIngredients();
    std::optional<json> getIngredientSubstitutes(int ingredientId, int limit);

    // User profile operations
    std::optional<json> getUserProfile(int userId);
    bool updateUser(int userId, const json& userData);
    bool deleteUser(int userId);

    // Favorite operations
    std::optional<json> getUserFavorites(int userId);
    bool addFavorite(int userId, int recipeId);
    bool removeFavorite(int userId, int recipeId);

    // Review operations
    std::optional<json> getRecipeReviews(int recipeId);
    std::optional<json> createReview(int recipeId, const json& reviewData);
    bool updateReview(int recipeId, int reviewId, const json& reviewData);
    bool deleteReview(int reviewId);

    // Admin operations
    std::optional<json> getAllUsers(int limit, int offset);
    bool updateUserRole(int userId, int role);
    bool adminDeleteUser(int userId);

private:
    std::string baseUrl_;
    std::string doRequest(const std::string& method, const std::string& path, const std::string& body = "");
};

#endif // CORE_SERVICE_CLIENT_H
