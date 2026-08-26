#ifndef DATA_SERVICE_CLIENT_H
#define DATA_SERVICE_CLIENT_H

#include "http_client.h"
#include "nlohmann/json.hpp"
#include <string>
#include <optional>
#include <vector>

using json = nlohmann::json;

class DataServiceClient {
public:
    DataServiceClient(const std::string& dataServiceUrl);

    // User operations
    std::optional<json> getUserById(int userId);
    std::optional<json> getUserByEmail(const std::string& email);
    std::vector<json> getAllUsers(int limit = 100, int offset = 0);
    std::optional<json> createUser(const json& userData);
    bool updateUser(int userId, const json& userData);
    bool deleteUser(int userId);
    bool updateUserRole(int userId, int role);
    bool verifyCredentials(const std::string& email, const std::string& passwordHash);

    // Recipe operations
    std::vector<json> getAllRecipes(int limit = 100, int offset = 0);
    std::optional<json> getRecipeById(int recipeId);
    std::optional<json> getRecipePreview(int recipeId);
    std::optional<json> getRecipeDetail(int recipeId);
    std::optional<json> createRecipe(const json& recipeData);
    bool updateRecipe(int recipeId, const json& recipeData);
    bool deleteRecipe(int recipeId);
    std::vector<json> getRecipeIngredients(int recipeId);
    bool addIngredientToRecipe(int recipeId, const json& ingredientData);
    bool removeIngredientFromRecipe(int recipeId, int ingredientId);
    std::vector<json> getRecipesByCategory(int categoryId);
    std::vector<json> getRecipesByIngredients(const std::vector<int>& ingredientIds);

    // Category operations
    std::vector<json> getAllCategories();
    bool addCategoryToRecipe(int recipeId, int categoryId);

    // Ingredient operations
    std::vector<json> getAllIngredients();
    std::optional<json> getIngredientById(int ingredientId);
    std::vector<json> getIngredientSubstitutes(int ingredientId, double threshold = 0.7, int limit = 3);
    std::vector<json> searchIngredients(const std::string& name);

    // Favorite operations
    std::vector<json> getFavoritesByUserId(int userId);
    bool addFavorite(int userId, int recipeId);
    bool removeFavorite(int userId, int recipeId);
    bool isFavorite(int userId, int recipeId);

    // Review operations
    std::vector<json> getReviewsByRecipeId(int recipeId);
    std::vector<json> getReviewsByUserId(int userId);
    std::optional<json> createReview(const json& reviewData);
    bool updateReview(int reviewId, const json& reviewData);
    bool deleteReview(int reviewId);
    double getAverageRating(int recipeId);

private:
    HttpClient client_;
};

#endif // DATA_SERVICE_CLIENT_H
