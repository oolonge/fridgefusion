#ifndef RECIPE_CONTROLLER_H
#define RECIPE_CONTROLLER_H

#include "crow.h"
#include "application/interfaces/processors/IRecipeExplorerProcessor.h"
#include "web_api/middleware/jwt_middleware.h"
#include <memory>

namespace web_api {

class RecipeController {
public:
    RecipeController(std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor,
                     std::shared_ptr<JWTMiddleware> jwtMiddleware);
    
    // Recipes
    crow::response getRecipes(const crow::request& req);
    crow::response getRecipeById(const crow::request& req, int id);
    crow::response createRecipe(const crow::request& req);
    crow::response updateRecipe(const crow::request& req, int id);
    crow::response patchRecipe(const crow::request& req, int id);
    crow::response deleteRecipe(const crow::request& req, int id);
    
    // Ingredients (через RecipeExplorerProcessor)
    crow::response getIngredients(const crow::request& req);
    crow::response getIngredientSubstitutes(const crow::request& req, int id);
    
    // ДОБАВЛЯЕМ: Categories (через RecipeExplorerProcessor)
    crow::response getCategories(const crow::request& req);

private:
    std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor_;
    std::shared_ptr<JWTMiddleware> jwtMiddleware_;
    
    QList<int> parseIngredientIds(const std::string& ingredientsParam);
};

} // namespace web_api

#endif // RECIPE_CONTROLLER_H