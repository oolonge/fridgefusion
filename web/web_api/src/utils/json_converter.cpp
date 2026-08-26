#include "web_api/utils/json_converter.h"

namespace web_api {

// ==================== USER CONVERSIONS ====================

crow::json::wvalue JsonConverter::userToJson(const User& user) {
    crow::json::wvalue json;
    json["id"] = user.getId();
    json["username"] = user.getUsername().toStdString();
    json["email"] = user.getEmail().toStdString();
    json["role"] = roleToString(user.getRole());
    json["registration_date"] = dateTimeToString(user.getRegistrationDate());
    return json;
}

crow::json::wvalue JsonConverter::userPublicToJson(const User& user) {
    crow::json::wvalue json;
    json["id"] = user.getId();
    json["username"] = user.getUsername().toStdString();
    return json;
}

crow::json::wvalue JsonConverter::userProfileToJson(const UserProfileDTO& profile) {
    crow::json::wvalue json;
    json["id"] = profile.id;
    json["username"] = profile.username.toStdString();
    json["email"] = profile.email.toStdString();
    json["role"] = roleToString(profile.role);
    json["registration_date"] = dateTimeToString(profile.registrationDate);
    json["favorites_count"] = profile.favoriteRecipesCount;
    json["reviews_count"] = profile.reviewsCount;
    return json;
}

crow::json::wvalue JsonConverter::authResultToJson(const AuthResult& result) {
    crow::json::wvalue json;
    json["success"] = result.isSuccess();
    
    if (!result.isSuccess()) {
        json["message"] = result.getErrorMessage().toStdString();
    } else {
        json["message"] = "Success";
    }
    
    if (result.getUser().has_value()) {
        json["user"] = userToJson(result.getUser().value());
    }
    
    return json;
}

// ==================== RECIPE CONVERSIONS ====================

crow::json::wvalue JsonConverter::recipeToJson(const Recipe& recipe) {
    crow::json::wvalue json;
    json["id"] = recipe.getRecipeId();
    json["name"] = recipe.getName().toStdString();
    json["description"] = recipe.getDescription().toStdString();
    json["preparation_time"] = recipe.getPreparationTime();
    json["cooking_time"] = recipe.getCookingTime();
    
    // Convert category IDs to JSON array
    std::vector<int> categoryIds;
    for (int catId : recipe.getCategoryIds()) {
        categoryIds.push_back(catId);
    }
    json["categories"] = categoryIds;
    
    return json;
}

crow::json::wvalue JsonConverter::recipePreviewToJson(const RecipePreviewDTO& preview) {
    crow::json::wvalue json;
    json["id"] = preview.id;
    json["name"] = preview.name.toStdString();
    json["short_description"] = preview.shortDescription.toStdString();
    json["total_time"] = preview.totalTime;
    json["average_rating"] = preview.averageRating;
    json["author_name"] = preview.authorName.toStdString();
    
    std::vector<std::string> ingredients;
    for (const auto& ing : preview.mainIngredients) {
        ingredients.push_back(ing.toStdString());
    }
    json["main_ingredients"] = ingredients;
    
    return json;
}

crow::json::wvalue JsonConverter::recipeDetailToJson(const RecipeDetailDTO& detail) {
    crow::json::wvalue json;
    json["id"] = detail.id;
    json["name"] = detail.name.toStdString();
    json["description"] = detail.fullDescription.toStdString();
    json["preparation_time"] = detail.preparationTime;
    json["cooking_time"] = detail.cookingTime;
    
    std::vector<std::string> categories;
    for (const auto& cat : detail.categories) {
        categories.push_back(cat.toStdString());
    }
    json["categories"] = categories;
    
    std::vector<std::string> instructions;
    for (const auto& step : detail.steps) {
        instructions.push_back(step.toStdString());
    }
    json["instructions"] = instructions;
    
    std::vector<crow::json::wvalue> ingredientsJson;
    for (const auto& ing : detail.ingredients) {
        crow::json::wvalue item;
        item["id"] = ing.id;
        item["name"] = ing.name.toStdString();
        item["quantity"] = ing.quantity;
        item["unit"] = ing.unit.toStdString();
        item["is_optional"] = ing.isOptional;
        
        if (!ing.possibleSubstitutes.isEmpty()) {
            std::vector<crow::json::wvalue> substitutesJson;
            for (const auto& sub : ing.possibleSubstitutes) {
                crow::json::wvalue subJson;
                subJson["id"] = sub.first;
                subJson["name"] = sub.second.toStdString();
                substitutesJson.push_back(std::move(subJson));
            }
            item["substitutes"] = std::move(substitutesJson);
        }
        
        ingredientsJson.push_back(std::move(item));
    }
    json["ingredients"] = std::move(ingredientsJson);
    
    json["average_rating"] = detail.averageRating;
    json["review_count"] = detail.reviewCount;
    json["is_favorite"] = detail.isFavorite;
    
    return json;
}

crow::json::wvalue JsonConverter::recipesListToJson(const std::vector<RecipePreviewDTO>& recipes,
                                                    int total, int limit, int offset) {
    crow::json::wvalue json;
    
    std::vector<crow::json::wvalue> recipesJson;
    for (const auto& recipe : recipes) {
        recipesJson.push_back(recipePreviewToJson(recipe));
    }
    
    json["recipes"] = std::move(recipesJson);
    json["total"] = total;
    json["limit"] = limit;
    json["offset"] = offset;
    
    return json;
}

crow::json::wvalue JsonConverter::recipeIngredientToJson(const RecipeIngredient& ri) {
    crow::json::wvalue json;
    // RecipeIngredient has simple getters, not complex objects
    json["ingredient_id"] = ri.getIngredientId();
    json["quantity"] = ri.getQuantity();
    json["unit"] = ri.getUnit().toStdString();
    json["is_optional"] = ri.isOptional();
    
    return json;
}

// ==================== INGREDIENT CONVERSIONS ====================

crow::json::wvalue JsonConverter::ingredientToJson(const Ingredient& ingredient) {
    crow::json::wvalue json;
    json["id"] = ingredient.getIngredientId();
    json["name"] = ingredient.getName().toStdString();
    json["calories"] = ingredient.getCalories();
    json["default_unit"] = ingredient.getDefaultUnit().toStdString();
    return json;
}

crow::json::wvalue JsonConverter::ingredientsListToJson(const std::vector<Ingredient>& ingredients) {
    std::vector<crow::json::wvalue> result;
    for (const auto& ingredient : ingredients) {
        result.push_back(ingredientToJson(ingredient));
    }
    return crow::json::wvalue(result);
}

// ==================== CATEGORY CONVERSIONS ====================

crow::json::wvalue JsonConverter::categoryToJson(const Category& category) {
    crow::json::wvalue json;
    json["id"] = category.getCategoryId();
    json["name"] = category.getName().toStdString();
    json["description"] = category.getDescription().toStdString();
    return json;
}

crow::json::wvalue JsonConverter::categoriesListToJson(const std::vector<Category>& categories) {
    std::vector<crow::json::wvalue> result;
    for (const auto& category : categories) {
        result.push_back(categoryToJson(category));
    }
    return crow::json::wvalue(result);
}

// ==================== FAVORITE CONVERSIONS ====================

crow::json::wvalue JsonConverter::favoriteToJson(const Favorite& favorite) {
    crow::json::wvalue json;
    json["id"] = favorite.getId();  
    json["recipe_id"] = favorite.getRecipeId();
    json["date_added"] = dateTimeToString(favorite.getDateAdded());
    return json;
}

crow::json::wvalue JsonConverter::favoritesListToJson(const std::vector<Favorite>& favorites) {
    std::vector<crow::json::wvalue> result;
    for (const auto& favorite : favorites) {
        result.push_back(favoriteToJson(favorite));
    }
    return crow::json::wvalue(result);
}

// ==================== REVIEW CONVERSIONS ====================

crow::json::wvalue JsonConverter::reviewToJson(const Review& review) {
    crow::json::wvalue json;
    json["id"] = review.getId();  
    json["user_id"] = review.getUserId();
    json["username"] = review.getUsername().toStdString();
    json["recipe_id"] = review.getRecipeId();
    json["rating"] = review.getRating();
    json["comment"] = review.getComment().toStdString();
    json["date_posted"] = dateTimeToString(review.getDatePosted());
    return json;
}

crow::json::wvalue JsonConverter::reviewsWithStatsToJson(const std::vector<Review>& reviews,
                                                        double averageRating, int totalCount) {
    crow::json::wvalue json;
    
    std::vector<crow::json::wvalue> reviewsJson;
    for (const auto& review : reviews) {
        reviewsJson.push_back(reviewToJson(review));
    }
    
    json["reviews"] = std::move(reviewsJson);
    json["average_rating"] = averageRating;
    json["total_count"] = totalCount;
    
    return json;
}

// ==================== HELPER METHODS ====================

std::string JsonConverter::roleToString(UserRole role) {
    switch (role) {
        case UserRole::USER: return "USER";
        case UserRole::ADMIN: return "ADMIN";
        case UserRole::GUEST: return "GUEST";
        default: return "USER";
    }
}

std::string JsonConverter::dateTimeToString(const QDateTime& dt) {
    return dt.toString(Qt::ISODate).toStdString();
}

double JsonConverter::sanitizeDouble(double value) {
    // Эта функция гарантирует, что double будет корректно сериализован в JSON
    // независимо от системной локали. Crow напрямую присваивает double,
    // поэтому просто возвращаем значение как есть.
    // Проблема решается установкой C-локали в main.cpp
    return value;
}

} // namespace web_api