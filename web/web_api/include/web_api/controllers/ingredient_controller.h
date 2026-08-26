#ifndef INGREDIENT_CONTROLLER_H
#define INGREDIENT_CONTROLLER_H

#include "crow.h"
#include "application/interfaces/services/IIngredientService.h"
#include <memory>

namespace web_api {

class IngredientController {
public:
    explicit IngredientController(std::shared_ptr<IIngredientService> ingredientService);
    
    // GET /ingredients
    crow::response getIngredients(const crow::request& req);
    
    // GET /ingredients/{id}/substitutes
    crow::response getSubstitutes(const crow::request& req, int id);

private:
    std::shared_ptr<IIngredientService> ingredientService_;
};

} // namespace web_api

#endif // INGREDIENT_CONTROLLER_H