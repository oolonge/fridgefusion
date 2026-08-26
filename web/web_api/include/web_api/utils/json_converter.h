#ifndef JSON_CONVERTER_H
#define JSON_CONVERTER_H

#include "crow.h"
#include "domain/entities/user.h"
#include "domain/entities/recipe.h"
#include "domain/entities/ingredient.h"
#include "domain/entities/category.h"
#include "domain/entities/favorite.h"
#include "domain/entities/review.h"
#include "domain/dtos/recipeDetailDTO.h"
#include "domain/dtos/recipePreviewDTO.h"
#include "domain/dtos/userProfileDTO.h"
#include "domain/dtos/authResult.h"
#include <vector>

namespace web_api {

class JsonConverter {
public:
    // User conversions
    static crow::json::wvalue userToJson(const User& user);
    static crow::json::wvalue userPublicToJson(const User& user);
    static crow::json::wvalue userProfileToJson(const UserProfileDTO& profile);
    static crow::json::wvalue authResultToJson(const AuthResult& result);
    
    // Recipe conversions
    static crow::json::wvalue recipeToJson(const Recipe& recipe);
    static crow::json::wvalue recipePreviewToJson(const RecipePreviewDTO& preview);
    static crow::json::wvalue recipeDetailToJson(const RecipeDetailDTO& detail);
    static crow::json::wvalue recipesListToJson(const std::vector<RecipePreviewDTO>& recipes,
                                                int total, int limit, int offset);
    
    // Ingredient conversions
    static crow::json::wvalue ingredientToJson(const Ingredient& ingredient);
    static crow::json::wvalue ingredientsListToJson(const std::vector<Ingredient>& ingredients);
    
    // Category conversions
    static crow::json::wvalue categoryToJson(const Category& category);
    static crow::json::wvalue categoriesListToJson(const std::vector<Category>& categories);
    
    // Favorite conversions
    static crow::json::wvalue favoriteToJson(const Favorite& favorite);
    static crow::json::wvalue favoritesListToJson(const std::vector<Favorite>& favorites);
    
    // Review conversions
    static crow::json::wvalue reviewToJson(const Review& review);
    static crow::json::wvalue reviewsWithStatsToJson(const std::vector<Review>& reviews,
                                                     double averageRating, int totalCount);
    
    // Helper for RecipeIngredient within RecipeDetail
    static crow::json::wvalue recipeIngredientToJson(const RecipeIngredient& ri);

    // Helper methods (public for use in controllers)
    static std::string dateTimeToString(const QDateTime& dt);

private:
    static std::string roleToString(UserRole role);

    // Вспомогательная функция для безопасной сериализации double в JSON
    // Гарантирует использование точки в качестве десятичного разделителя
    static double sanitizeDouble(double value);
};

} // namespace web_api

#endif // JSON_CONVERTER_H