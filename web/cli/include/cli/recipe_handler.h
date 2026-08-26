#ifndef RECIPE_HANDLER_H
#define RECIPE_HANDLER_H

#include <memory>
#include <string>
#include <vector>

#include "cli/config_manager.h"
#include "cli/console_view.h"
#include "cli/http_client.h"

namespace cli {

// Обработчик рецептов
class RecipeHandler {
 public:
  RecipeHandler(std::shared_ptr<HttpClient> httpClient,
                std::shared_ptr<ConfigManager> config,
                std::shared_ptr<ConsoleView> view);
  ~RecipeHandler();

  // Получение рецептов
  std::vector<RecipePreview> getAllRecipes(int limit = 20, int offset = 0);
  RecipeDetail getRecipeDetails(int recipeId);
  std::vector<RecipePreview> searchByIngredients(
      const std::vector<int>& ingredientIds);
  std::vector<RecipePreview> searchByCategory(int categoryId);

  // Получение справочных данных
  std::vector<Ingredient> getAllIngredients();
  std::vector<Category> getAllCategories();
  std::vector<Review> getRecipeReviews(int recipeId);

 private:
  std::shared_ptr<HttpClient> httpClient_;
  std::shared_ptr<ConfigManager> config_;
  std::shared_ptr<ConsoleView> view_;

  void handleError(const HttpResponse& response);
};

}  // namespace cli

#endif  // RECIPE_HANDLER_H
