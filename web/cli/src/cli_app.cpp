#include "cli/cli_app.h"

#include <algorithm>

namespace cli {

CLIApp::CLIApp(std::shared_ptr<ConfigManager> config)
    : config_(config), running_(true) {
  // Инициализируем HTTP-клиент
  httpClient_ = std::make_shared<HttpClient>(config->getApiUrl());

  // Инициализируем view
  view_ = std::make_shared<ConsoleView>();

  // Инициализируем handlers
  authHandler_ =
      std::make_shared<AuthHandler>(httpClient_, config_, view_);
  recipeHandler_ =
      std::make_shared<RecipeHandler>(httpClient_, config_, view_);
  favoriteHandler_ =
      std::make_shared<FavoriteHandler>(httpClient_, config_, view_);
  userHandler_ =
      std::make_shared<UserHandler>(httpClient_, config_, view_);
}

CLIApp::~CLIApp() {}

void CLIApp::run() {
  view_->displayMessage("Добро пожаловать в FridgeFusion CLI!");

  while (running_) {
    if (authHandler_->isLoggedIn()) {
      showMainMenu();
    } else {
      showAuthMenu();
    }
  }

  view_->displayMessage("До свидания!");
}

void CLIApp::showAuthMenu() {
  std::vector<std::string> options = {"Войти", "Зарегистрироваться",
                                      "Выйти из программы"};

  int choice = view_->showMenu("АВТОРИЗАЦИЯ", options);

  switch (choice) {
    case 0:
      login();
      break;
    case 1:
      registerUser();
      break;
    case 2:
      running_ = false;
      break;
  }
}

void CLIApp::showMainMenu() {
  std::vector<std::string> options = {
      "Просмотр рецептов",
      "Поиск рецептов по ингредиентам",
      "Поиск рецептов по категории",
      "Мой профиль",
      "Избранные рецепты",
      "Выйти из аккаунта",
      "Выйти из программы"};

  int choice = view_->showMenu("ГЛАВНОЕ МЕНЮ", options);

  switch (choice) {
    case 0:
      showAllRecipes();
      break;
    case 1:
      searchRecipesByIngredients();
      break;
    case 2:
      searchRecipesByCategory();
      break;
    case 3:
      showUserProfile();
      break;
    case 4:
      showFavoriteRecipes();
      break;
    case 5:
      logout();
      break;
    case 6:
      running_ = false;
      break;
  }
}

void CLIApp::login() {
  view_->displayMessage("=== ВХОД В АККАУНТ ===");
  std::string email = view_->getInput("Email");
  std::string password = view_->getInput("Пароль");

  authHandler_->login(email, password);
  view_->getInput("\nНажмите Enter для продолжения");
}

void CLIApp::registerUser() {
  view_->displayMessage("=== РЕГИСТРАЦИЯ ===");
  std::string username = view_->getInput("Имя пользователя");
  std::string email = view_->getInput("Email");
  std::string password = view_->getInput("Пароль");
  std::string confirmPassword = view_->getInput("Подтвердите пароль");

  if (password != confirmPassword) {
    view_->displayError("Пароли не совпадают");
    view_->getInput("\nНажмите Enter для продолжения");
    return;
  }

  authHandler_->registerUser(username, email, password);
  view_->getInput("\nНажмите Enter для продолжения");
}

void CLIApp::logout() {
  authHandler_->logout();
  view_->getInput("\nНажмите Enter для продолжения");
}

void CLIApp::showAllRecipes() {
  auto recipes = recipeHandler_->getAllRecipes(20, 0);
  view_->displayRecipes(recipes);

  if (!recipes.empty()) {
    int recipeId =
        view_->getIntInput("Введите ID рецепта для просмотра деталей (0 - вернуться)", 0);
    if (recipeId > 0) {
      showRecipeDetails(recipeId);
    }
  } else {
    view_->getInput("Нажмите Enter для продолжения");
  }
}

void CLIApp::searchRecipesByIngredients() {
  // Получаем список ингредиентов
  auto allIngredients = recipeHandler_->getAllIngredients();
  view_->displayIngredients(allIngredients);

  // Выбираем ингредиенты
  std::vector<int> selectedIngredients = selectIngredients();

  if (selectedIngredients.empty()) {
    view_->displayMessage("Поиск отменен");
    view_->getInput("\nНажмите Enter для продолжения");
    return;
  }

  // Ищем рецепты
  auto recipes = recipeHandler_->searchByIngredients(selectedIngredients);
  view_->displayRecipes(recipes);

  if (!recipes.empty()) {
    int recipeId =
        view_->getIntInput("Введите ID рецепта для просмотра деталей (0 - вернуться)", 0);
    if (recipeId > 0) {
      showRecipeDetails(recipeId);
    }
  } else {
    view_->getInput("Нажмите Enter для продолжения");
  }
}

void CLIApp::searchRecipesByCategory() {
  // Получаем список категорий
  auto allCategories = recipeHandler_->getAllCategories();
  view_->displayCategories(allCategories);

  // Выбираем категорию
  int categoryId = selectCategory();

  if (categoryId == 0) {
    view_->displayMessage("Поиск отменен");
    view_->getInput("\nНажмите Enter для продолжения");
    return;
  }

  // Ищем рецепты
  auto recipes = recipeHandler_->searchByCategory(categoryId);
  view_->displayRecipes(recipes);

  if (!recipes.empty()) {
    int recipeId =
        view_->getIntInput("Введите ID рецепта для просмотра деталей (0 - вернуться)", 0);
    if (recipeId > 0) {
      showRecipeDetails(recipeId);
    }
  } else {
    view_->getInput("Нажмите Enter для продолжения");
  }
}

void CLIApp::showRecipeDetails(int recipeId) {
  // Получаем детали рецепта
  auto recipe = recipeHandler_->getRecipeDetails(recipeId);

  if (recipe.id == 0) {
    view_->displayError("Рецепт не найден");
    view_->getInput("\nНажмите Enter для продолжения");
    return;
  }

  // Отображаем детали рецепта
  view_->displayRecipeDetails(recipe);

  // Отображаем отзывы
  auto reviews = recipeHandler_->getRecipeReviews(recipeId);
  view_->displayReviews(reviews, recipe.average_rating);

  // Меню действий с рецептом
  std::vector<std::string> options;
  if (authHandler_->isLoggedIn()) {
    bool isFav = favoriteHandler_->isFavorite(recipeId);
    options.push_back(isFav ? "Удалить из избранного" : "Добавить в избранное");
  }
  options.push_back("Вернуться назад");

  view_->displayMessage("\nДоступные действия:");
  for (size_t i = 0; i < options.size(); i++) {
    view_->displayMessage(std::to_string(i + 1) + ". " + options[i]);
  }

  int choice = view_->getIntInput("Введите номер действия", 1,
                                   static_cast<int>(options.size())) - 1;

  if (authHandler_->isLoggedIn() && choice == 0) {
    toggleFavorite(recipeId);
    showRecipeDetails(recipeId);  // Обновляем экран
  }
}

void CLIApp::showFavoriteRecipes() {
  if (!authHandler_->isLoggedIn()) {
    view_->displayError("Необходимо авторизоваться");
    view_->getInput("\nНажмите Enter для продолжения");
    return;
  }

  auto recipes = favoriteHandler_->getFavorites();

  if (recipes.empty()) {
    view_->displayMessage("У вас нет избранных рецептов");
    view_->getInput("\nНажмите Enter для продолжения");
    return;
  }

  view_->displayRecipes(recipes);

  int recipeId =
      view_->getIntInput("Введите ID рецепта для просмотра деталей (0 - вернуться)", 0);
  if (recipeId > 0) {
    showRecipeDetails(recipeId);
  }
}

void CLIApp::toggleFavorite(int recipeId) {
  if (!authHandler_->isLoggedIn()) {
    view_->displayError("Необходимо авторизоваться");
    return;
  }

  bool isFavorite = favoriteHandler_->isFavorite(recipeId);

  if (isFavorite) {
    favoriteHandler_->removeFavorite(recipeId);
  } else {
    favoriteHandler_->addFavorite(recipeId);
  }
}

void CLIApp::showUserProfile() {
  if (!authHandler_->isLoggedIn()) {
    view_->displayError("Необходимо авторизоваться");
    view_->getInput("\nНажмите Enter для продолжения");
    return;
  }

  auto profile = userHandler_->getProfile();

  if (profile.id == 0) {
    view_->displayError("Не удалось загрузить профиль пользователя");
    view_->getInput("\nНажмите Enter для продолжения");
    return;
  }

  view_->displayUserProfile(profile);

  std::vector<std::string> options = {"Показать избранные рецепты",
                                      "Вернуться в главное меню"};

  view_->displayMessage("\nДоступные действия:");
  for (size_t i = 0; i < options.size(); i++) {
    view_->displayMessage(std::to_string(i + 1) + ". " + options[i]);
  }

  int choice = view_->getIntInput("Выберите действие", 1,
                                   static_cast<int>(options.size())) - 1;

  if (choice == 0) {
    showFavoriteRecipes();
  }
}

std::vector<int> CLIApp::selectIngredients() {
  std::vector<int> selectedIds;

  while (true) {
    int id = view_->getIntInput(
        "Введите ID ингредиента для добавления (0 - завершить выбор)", 0);

    if (id == 0) {
      break;
    }

    // Проверяем, не был ли уже добавлен
    if (std::find(selectedIds.begin(), selectedIds.end(), id) ==
        selectedIds.end()) {
      selectedIds.push_back(id);
      view_->displayMessage("Ингредиент добавлен. Всего выбрано: " +
                            std::to_string(selectedIds.size()));
    } else {
      view_->displayMessage("Этот ингредиент уже выбран");
    }
  }

  return selectedIds;
}

int CLIApp::selectCategory() {
  int id = view_->getIntInput("Введите ID категории (0 - отмена)", 0);
  return id;
}

}  // namespace cli
