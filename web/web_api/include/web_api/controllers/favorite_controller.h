#ifndef FAVORITE_CONTROLLER_H
#define FAVORITE_CONTROLLER_H

#include "crow.h"
#include "application/interfaces/processors/IFavoriteProcessor.h"
#include "application/interfaces/processors/IRecipeExplorerProcessor.h"
#include "web_api/middleware/jwt_middleware.h"
#include <memory>

namespace web_api {

class FavoriteController {
public:
    FavoriteController(std::shared_ptr<IFavoriteProcessor> favoriteProcessor,
                       std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor,
                       std::shared_ptr<JWTMiddleware> jwtMiddleware);

    // GET /favorites
    crow::response getFavorites(const crow::request& req);

    // POST /favorites (with recipe_id in body)
    crow::response addFavorite(const crow::request& req);

    // DELETE /favorites/{recipe_id}
    crow::response removeFavorite(const crow::request& req, int recipeId);

private:
    std::shared_ptr<IFavoriteProcessor> favoriteProcessor_;
    std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor_;
    std::shared_ptr<JWTMiddleware> jwtMiddleware_;
};

} // namespace web_api

#endif // FAVORITE_CONTROLLER_H