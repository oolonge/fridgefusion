#ifndef CLI_APP_H
#define CLI_APP_H

#include <memory>

#include "cli/auth_handler.h"
#include "cli/config_manager.h"
#include "cli/console_view.h"
#include "cli/favorite_handler.h"
#include "cli/http_client.h"
#include "cli/recipe_handler.h"
#include "cli/user_handler.h"

namespace cli {

// Главный класс CLI приложения
class CLIApp {
 public:
  explicit CLIApp(std::shared_ptr<ConfigManager> config);
  ~CLIApp();

  // Главный цикл приложения
  void run();

 private:
  // Компоненты
  std::shared_ptr<ConfigManager> config_;
  std::shared_ptr<HttpClient> httpClient_;
  std::shared_ptr<ConsoleView> view_;
  std::shared_ptr<AuthHandler> authHandler_;
  std::shared_ptr<RecipeHandler> recipeHandler_;
  std::shared_ptr<FavoriteHandler> favoriteHandler_;
  std::shared_ptr<UserHandler> userHandler_;

  // Состояние приложения
  bool running_;

  // Меню
  void showAuthMenu();
  void showMainMenu();

  // Авторизация
  void login();
  void registerUser();
  void logout();

  // Рецепты
  void showAllRecipes();
  void searchRecipesByIngredients();
  void searchRecipesByCategory();
  void showRecipeDetails(int recipeId);

  // Избранное
  void showFavoriteRecipes();
  void toggleFavorite(int recipeId);

  // Профиль
  void showUserProfile();

  // Вспомогательные методы
  std::vector<int> selectIngredients();
  int selectCategory();
};

}  // namespace cli

#endif  // CLI_APP_H
