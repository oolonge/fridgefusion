#include "web_api/controllers/favorite_controller.h"
#include "web_api/utils/response_builder.h"
#include "web_api/utils/json_converter.h"

namespace web_api {

FavoriteController::FavoriteController(std::shared_ptr<IFavoriteProcessor> favoriteProcessor,
                                       std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor,
                                       std::shared_ptr<JWTMiddleware> jwtMiddleware)
    : favoriteProcessor_(favoriteProcessor), recipeProcessor_(recipeProcessor), jwtMiddleware_(jwtMiddleware) {
}

crow::response FavoriteController::getFavorites(const crow::request& req) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }

        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }

        // Get user's favorites
        auto favorites = favoriteProcessor_->getUserFavorites(userId.value());

        // Build response with recipe names
        std::vector<crow::json::wvalue> result;
        for (const auto& fav : favorites) {
            crow::json::wvalue item;
            item["id"] = fav.getId();
            item["recipe_id"] = fav.getRecipeId();
            item["date_added"] = JsonConverter::dateTimeToString(fav.getDateAdded());

            // Get recipe name through RecipeProcessor
            auto recipePreview = recipeProcessor_->getRecipePreview(fav.getRecipeId());
            item["recipe_name"] = recipePreview.name.toStdString();

            result.push_back(std::move(item));
        }

        return ResponseBuilder::ok(crow::json::wvalue(result));

    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response FavoriteController::addFavorite(const crow::request& req) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        // Parse request body
        auto json = crow::json::load(req.body);
        if (!json) {
            return ResponseBuilder::badRequest("Invalid JSON");
        }
        
        if (!json.has("recipe_id")) {
            return ResponseBuilder::badRequest("recipe_id is required");
        }
        
        int recipeId = json["recipe_id"].i();
        
        // Add to favorites
        bool success = favoriteProcessor_->addToFavorites(userId.value(), recipeId);
        
        if (!success) {
            return ResponseBuilder::badRequest("Failed to add to favorites (recipe may not exist or already in favorites)");
        }
        
        crow::json::wvalue responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "Recipe added to favorites";
        responseJson["recipe_id"] = recipeId;
        
        return ResponseBuilder::created(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response FavoriteController::removeFavorite(const crow::request& req, int recipeId) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        // Remove from favorites
        bool success = favoriteProcessor_->removeFromFavorites(userId.value(), recipeId);
        
        if (!success) {
            return ResponseBuilder::notFound("Favorite not found");
        }
        
        return ResponseBuilder::noContent();
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

} // namespace web_api