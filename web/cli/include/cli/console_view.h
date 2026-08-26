#ifndef CONSOLE_VIEW_H
#define CONSOLE_VIEW_H

#include <string>
#include <vector>

namespace cli {

// Структуры данных для отображения

struct RecipeIngredient {
  std::string name;
  double quantity;
  std::string unit;
  bool is_optional;
};

struct RecipePreview {
  int id;
  std::string name;
  std::string description;
  int total_time;
  double average_rating;
};

struct RecipeDetail {
  int id;
  std::string name;
  std::string description;
  int preparation_time;
  int cooking_time;
  std::vector<std::string> instructions;
  std::vector<RecipeIngredient> ingredients;
  std::vector<std::string> categories;
  double average_rating;
  int review_count;
};

struct UserProfile {
  int id;
  std::string username;
  std::string email;
  int favorites_count;
  int reviews_count;
  int recipes_count;
};

struct Category {
  int id;
  std::string name;
  std::string description;
};

struct Ingredient {
  int id;
  std::string name;
};

struct Review {
  int id;
  std::string username;
  int rating;
  std::string comment;
  std::string created_at;
};

// Класс для работы с консольным вводом/выводом
class ConsoleView {
 public:
  ConsoleView();
  ~ConsoleView();

  // Вывод сообщений
  void displayMessage(const std::string& message);
  void displayError(const std::string& error);
  void displaySuccess(const std::string& message);

  // Ввод данных
  std::string getInput(const std::string& prompt);
  int getIntInput(const std::string& prompt, int min = 0, int max = INT_MAX);
  bool getBoolInput(const std::string& prompt);

  // Меню
  int showMenu(const std::string& title,
               const std::vector<std::string>& options);

  // Вывод данных о рецептах
  void displayRecipes(const std::vector<RecipePreview>& recipes);
  void displayRecipeDetails(const RecipeDetail& recipe);
  void displayCategories(const std::vector<Category>& categories);
  void displayIngredients(const std::vector<Ingredient>& ingredients);
  void displayReviews(const std::vector<Review>& reviews, double averageRating);

  // Вывод данных о пользователе
  void displayUserProfile(const UserProfile& profile);

  // Очистка консоли
  void clearScreen();
};

}  // namespace cli

#endif  // CONSOLE_VIEW_H
