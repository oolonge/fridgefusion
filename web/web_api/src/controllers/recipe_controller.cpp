#include "web_api/controllers/recipe_controller.h"
#include "web_api/utils/request_validator.h"
#include "web_api/utils/response_builder.h"
#include "web_api/utils/json_converter.h"
#include <sstream>

namespace web_api {

RecipeController::RecipeController(std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor,
                                   std::shared_ptr<JWTMiddleware> jwtMiddleware)
    : recipeProcessor_(recipeProcessor), jwtMiddleware_(jwtMiddleware) {
}

crow::response RecipeController::getRecipes(const crow::request& req) {
    try {
        // Parse query parameters
        int limit = 20;
        int offset = 0;
        std::optional<int> categoryId;
        QList<int> ingredientIds;
        
        auto limitParam = req.url_params.get("limit");
        if (limitParam) {
            limit = std::stoi(limitParam);
        }
        
        auto offsetParam = req.url_params.get("offset");
        if (offsetParam) {
            offset = std::stoi(offsetParam);
        }
        
        // Validate pagination
        auto validation = RequestValidator::validatePagination(limit, offset);
        if (!validation.isValid) {
            return ResponseBuilder::badRequest(validation.message);
        }
        
        auto categoryParam = req.url_params.get("category_id");
        if (categoryParam) {
            categoryId = std::stoi(categoryParam);
        }
        
        auto ingredientsParam = req.url_params.get("ingredients");
        if (ingredientsParam) {
            ingredientIds = parseIngredientIds(ingredientsParam);
        }
        
        // Get recipes based on filters
        QList<RecipePreviewDTO> recipes;
        int totalCount = 0;
        
        if (!ingredientIds.isEmpty() && categoryId.has_value()) {
            // Search by both ingredients and category
            auto fullRecipes = recipeProcessor_->findRecipesByCategoryAndIngredients(
                categoryId.value(), ingredientIds
            );
            // Convert to previews
            for (int i = offset; i < offset + limit && i < fullRecipes.size(); ++i) {
                recipes.append(recipeProcessor_->getRecipePreview(fullRecipes[i].getRecipeId()));
            }
            totalCount = fullRecipes.size();
        } else if (!ingredientIds.isEmpty()) {
            // Search by ingredients only
            auto fullRecipes = recipeProcessor_->findRecipesWithIngredients(ingredientIds);
            for (int i = offset; i < offset + limit && i < fullRecipes.size(); ++i) {
                recipes.append(recipeProcessor_->getRecipePreview(fullRecipes[i].getRecipeId()));
            }
            totalCount = fullRecipes.size();
        } else if (categoryId.has_value()) {
            // Filter by category only
            auto fullRecipes = recipeProcessor_->findRecipesByCategory(categoryId.value());
            for (int i = offset; i < offset + limit && i < fullRecipes.size(); ++i) {
                recipes.append(recipeProcessor_->getRecipePreview(fullRecipes[i].getRecipeId()));
            }
            totalCount = fullRecipes.size();
        } else {
            // Get all recipes
            recipes = recipeProcessor_->getAllRecipePreviews(limit, offset);
            // NOTE: We don't have getTotalRecipesCount(), so we approximate
            totalCount = recipes.size() == limit ? offset + limit + 1 : offset + recipes.size();
        }
        
        // Convert to vector for JsonConverter
        std::vector<RecipePreviewDTO> recipesVec;
        for (const auto& recipe : recipes) {
            recipesVec.push_back(recipe);
        }
        
        auto responseJson = JsonConverter::recipesListToJson(recipesVec, totalCount, limit, offset);
        return ResponseBuilder::ok(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response RecipeController::getRecipeById(const crow::request&, int id) {
    try {
        // Get recipe details (no userId parameter needed)
        auto recipeDetail = recipeProcessor_->getRecipeDetail(id);
        
        // Check if recipe exists by validating the returned data
        if (recipeDetail.id == 0) {
            return ResponseBuilder::notFound("Recipe not found");
        }
        
        auto responseJson = JsonConverter::recipeDetailToJson(recipeDetail);
        return ResponseBuilder::ok(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response RecipeController::createRecipe(const crow::request& req) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        auto json = crow::json::load(req.body);
        if (!json) {
            return ResponseBuilder::badRequest("Invalid JSON");
        }
        
        // Validate input
        auto validation = RequestValidator::validateRecipeCreate(json);
        if (!validation.isValid) {
            crow::json::wvalue details;
            details[validation.field] = validation.message;
            return ResponseBuilder::badRequest("Validation failed", details);
        }
        
        // NOTE: IRecipeExplorerProcessor doesn't have createRecipe method
        // This functionality should be implemented via IRecipeRepository or IAdminProcessor
        // For now, return not implemented
        
        crow::json::wvalue responseJson;
        responseJson["error"] = "NotImplemented";
        responseJson["message"] = "Recipe creation is not yet implemented in this API version";
        
        return ResponseBuilder::error(501, "Not Implemented", 
                                      "Recipe creation endpoint is not yet available");
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response RecipeController::updateRecipe(const crow::request& req, int) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        auto json = crow::json::load(req.body);
        if (!json) {
            return ResponseBuilder::badRequest("Invalid JSON");
        }
        
        // Validate input
        auto validation = RequestValidator::validateRecipeUpdate(json);
        if (!validation.isValid) {
            crow::json::wvalue details;
            details[validation.field] = validation.message;
            return ResponseBuilder::badRequest("Validation failed", details);
        }
        
        // NOTE: Update operations not available in IRecipeExplorerProcessor
        return ResponseBuilder::error(501, "Not Implemented",
                                      "Recipe update endpoint is not yet available");
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response RecipeController::patchRecipe(const crow::request& req, int) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        auto json = crow::json::load(req.body);
        if (!json) {
            return ResponseBuilder::badRequest("Invalid JSON");
        }
        
        // Validate input
        auto validation = RequestValidator::validateRecipePatch(json);
        if (!validation.isValid) {
            crow::json::wvalue details;
            details[validation.field] = validation.message;
            return ResponseBuilder::badRequest("Validation failed", details);
        }
        
        // NOTE: Partial update not available in IRecipeExplorerProcessor
        return ResponseBuilder::error(501, "Not Implemented",
                                      "Recipe partial update endpoint is not yet available");
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response RecipeController::deleteRecipe(const crow::request& req, int) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        // NOTE: Delete operations not available in IRecipeExplorerProcessor
        return ResponseBuilder::error(501, "Not Implemented",
                                      "Recipe deletion endpoint is not yet available");
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

// ==================== INGREDIENT METHODS ====================

crow::response RecipeController::getIngredients(const crow::request& req) {
    try {
        // Get optional search parameter
        auto searchParam = req.url_params.get("search");
        QList<Ingredient> ingredients;
        
        if (searchParam) {
            // Simple filtering by name
            QString searchQuery = QString::fromStdString(searchParam).toLower();
            auto allIngredients = recipeProcessor_->getAllIngredients();
            
            for (const auto& ing : allIngredients) {
                if (ing.getName().toLower().contains(searchQuery)) {
                    ingredients.append(ing);
                }
            }
        } else {
            ingredients = recipeProcessor_->getAllIngredients();
        }
        
        // Convert to vector
        std::vector<Ingredient> ingredientsVec;
        for (const auto& ing : ingredients) {
            ingredientsVec.push_back(ing);
        }
        
        auto responseJson = JsonConverter::ingredientsListToJson(ingredientsVec);
        return ResponseBuilder::ok(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response RecipeController::getIngredientSubstitutes(const crow::request& req, int id) {
    try {
        // Get optional limit parameter
        int limit = 5;
        auto limitParam = req.url_params.get("limit");
        if (limitParam) {
            limit = std::stoi(limitParam);
            if (limit < 1 || limit > 20) {
                return ResponseBuilder::badRequest("Limit must be between 1 and 20");
            }
        }
        
        // Get substitutes with similarity scores through processor
        auto substitutePairs = recipeProcessor_->getIngredientSubstitutes(id, limit);
        
        if (substitutePairs.isEmpty()) {
            return ResponseBuilder::notFound("Ingredient not found or no substitutes available");
        }
        
        // Build response with similarity scores
        std::vector<crow::json::wvalue> result;
        auto allIngredients = recipeProcessor_->getAllIngredients();
        
        for (const auto& pair : substitutePairs) {
            int substituteId = pair.first;
            double similarity = pair.second;
            
            // Find the ingredient by ID
            for (const auto& ing : allIngredients) {
                if (ing.getIngredientId() == substituteId) {
                    crow::json::wvalue item;
                    item["ingredient"] = JsonConverter::ingredientToJson(ing);
                    item["similarity"] = similarity;
                    result.push_back(std::move(item));
                    break;
                }
            }
        }
        
        return ResponseBuilder::ok(crow::json::wvalue(result));
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

// ==================== CATEGORY METHOD ====================

crow::response RecipeController::getCategories(const crow::request&) {
    try {
        // Get all categories through processor
        auto categories = recipeProcessor_->getAllCategories();
        
        // Convert to vector
        std::vector<Category> categoriesVec;
        for (const auto& cat : categories) {
            categoriesVec.push_back(cat);
        }
        
        auto responseJson = JsonConverter::categoriesListToJson(categoriesVec);
        return ResponseBuilder::ok(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

// ==================== HELPER METHODS ====================

QList<int> RecipeController::parseIngredientIds(const std::string& ingredientsParam) {
    QList<int> result;
    std::stringstream ss(ingredientsParam);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        try {
            result.append(std::stoi(item));
        } catch (...) {
            // Skip invalid IDs
        }
    }
    
    return result;
}

} // namespace web_api