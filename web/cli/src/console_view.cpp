#include "cli/console_view.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

namespace cli {

// Подсчёт количества UTF-8 символов (не байтов)
static size_t utf8Length(const std::string& str) {
  size_t len = 0;
  for (size_t i = 0; i < str.size(); ++i) {
    // Пропускаем continuation bytes (10xxxxxx)
    if ((str[i] & 0xC0) != 0x80) {
      ++len;
    }
  }
  return len;
}

// Обрезка UTF-8 строки по количеству символов
static std::string utf8Substr(const std::string& str, size_t maxChars) {
  size_t chars = 0;
  size_t bytePos = 0;

  while (bytePos < str.size() && chars < maxChars) {
    unsigned char c = str[bytePos];
    size_t charBytes = 1;

    // Определяем количество байт в символе
    if ((c & 0x80) == 0) {
      charBytes = 1;  // ASCII
    } else if ((c & 0xE0) == 0xC0) {
      charBytes = 2;  // 2-byte UTF-8
    } else if ((c & 0xF0) == 0xE0) {
      charBytes = 3;  // 3-byte UTF-8
    } else if ((c & 0xF8) == 0xF0) {
      charBytes = 4;  // 4-byte UTF-8
    }

    bytePos += charBytes;
    ++chars;
  }

  return str.substr(0, bytePos);
}

// Дополнение строки пробелами с учётом UTF-8
static std::string utf8PadRight(const std::string& str, size_t width) {
  size_t charLen = utf8Length(str);
  if (charLen >= width) {
    return str;
  }
  return str + std::string(width - charLen, ' ');
}

ConsoleView::ConsoleView() {}

ConsoleView::~ConsoleView() {}

void ConsoleView::displayMessage(const std::string& message) {
  std::cout << message << std::endl;
}

void ConsoleView::displayError(const std::string& error) {
  std::cout << "ОШИБКА: " << error << std::endl;
}

void ConsoleView::displaySuccess(const std::string& message) {
  std::cout << "УСПЕХ: " << message << std::endl;
}

std::string ConsoleView::getInput(const std::string& prompt) {
  std::cout << prompt << ": ";
  std::cout.flush();

  std::string input;
  std::getline(std::cin, input);

  return input;
}

int ConsoleView::getIntInput(const std::string& prompt, int min, int max) {
  while (true) {
    std::cout << prompt << ": ";
    std::cout.flush();

    int value;
    std::cin >> value;

    // Очищаем буфер после ввода
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (value >= min && value <= max) {
      return value;
    }

    std::cout << "ОШИБКА: Введите число от " << min << " до " << max
              << std::endl;
  }
}

bool ConsoleView::getBoolInput(const std::string& prompt) {
  while (true) {
    std::cout << prompt << " (д/н): ";
    std::cout.flush();

    std::string input;
    std::getline(std::cin, input);

    if (input == "д" || input == "да" || input == "y" || input == "yes") {
      return true;
    } else if (input == "н" || input == "нет" || input == "n" ||
               input == "no") {
      return false;
    }

    std::cout << "ОШИБКА: Введите 'д' или 'н'" << std::endl;
  }
}

int ConsoleView::showMenu(const std::string& title,
                          const std::vector<std::string>& options) {
  clearScreen();
  std::cout << "=== " << title << " ===" << std::endl;

  for (size_t i = 0; i < options.size(); ++i) {
    std::cout << i + 1 << ". " << options[i] << std::endl;
  }

  return getIntInput("Выберите опцию", 1, static_cast<int>(options.size())) -
         1;
}

void ConsoleView::displayRecipes(const std::vector<RecipePreview>& recipes) {
  if (recipes.empty()) {
    displayMessage("Рецепты не найдены");
    return;
  }

  std::cout << "Найдено рецептов: " << recipes.size() << std::endl;
  std::cout << "----------------------------------------------------------------------"
            << std::endl;
  // Заголовок с ручным выравниванием
  std::cout << utf8PadRight("ID", 5) << " | "
            << utf8PadRight("Название", 30) << " | "
            << utf8PadRight("Время", 8) << " | "
            << utf8PadRight("Рейтинг", 8) << std::endl;
  std::cout << "----------------------------------------------------------------------"
            << std::endl;

  for (const auto& recipe : recipes) {
    std::string name = recipe.name;
    // Обрезаем по UTF-8 символам, а не байтам
    if (utf8Length(name) > 30) {
      name = utf8Substr(name, 27) + "...";
    }

    // Форматируем время и рейтинг
    std::string timeStr = std::to_string(recipe.total_time);
    std::ostringstream ratingStream;
    ratingStream << std::fixed << std::setprecision(1) << recipe.average_rating;
    std::string ratingStr = ratingStream.str();

    // Выводим с ручным выравниванием для UTF-8
    std::cout << utf8PadRight(std::to_string(recipe.id), 5) << " | "
              << utf8PadRight(name, 30) << " | "
              << utf8PadRight(timeStr, 8) << " | "
              << utf8PadRight(ratingStr, 8) << std::endl;
  }
  std::cout << "----------------------------------------------------------------------"
            << std::endl;
}

void ConsoleView::displayRecipeDetails(const RecipeDetail& recipe) {
  clearScreen();
  std::cout << "=== РЕЦЕПТ: " << recipe.name << " ===" << std::endl;
  std::cout << recipe.description << std::endl << std::endl;

  std::cout << "Время подготовки: " << recipe.preparation_time << " мин"
            << std::endl;
  std::cout << "Время приготовления: " << recipe.cooking_time << " мин"
            << std::endl;
  std::cout << "Общее время: "
            << (recipe.preparation_time + recipe.cooking_time) << " мин"
            << std::endl;

  // Категории
  if (!recipe.categories.empty()) {
    std::cout << "Категории: ";
    for (size_t i = 0; i < recipe.categories.size(); ++i) {
      std::cout << recipe.categories[i];
      if (i < recipe.categories.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
  }

  std::cout << "Рейтинг: " << std::fixed << std::setprecision(1)
            << recipe.average_rating << " (" << recipe.review_count
            << " отзывов)" << std::endl
            << std::endl;

  // Ингредиенты
  std::cout << "Ингредиенты:" << std::endl;
  for (const auto& ingredient : recipe.ingredients) {
    std::cout << "• " << ingredient.name << " - " << ingredient.quantity << " "
              << ingredient.unit;
    if (ingredient.is_optional) {
      std::cout << " (опционально)";
    }
    std::cout << std::endl;
  }

  // Инструкции
  std::cout << std::endl << "Инструкция:" << std::endl;
  for (size_t i = 0; i < recipe.instructions.size(); ++i) {
    std::cout << i + 1 << ". " << recipe.instructions[i] << std::endl;
  }
}

void ConsoleView::displayCategories(const std::vector<Category>& categories) {
  if (categories.empty()) {
    displayMessage("Категории не найдены");
    return;
  }

  std::cout << "Доступные категории:" << std::endl;
  for (const auto& category : categories) {
    std::cout << category.id << ". " << category.name;
    if (!category.description.empty()) {
      std::cout << " - " << category.description;
    }
    std::cout << std::endl;
  }
}

void ConsoleView::displayIngredients(
    const std::vector<Ingredient>& ingredients) {
  if (ingredients.empty()) {
    displayMessage("Ингредиенты не найдены");
    return;
  }

  std::cout << "Доступные ингредиенты:" << std::endl;
  for (const auto& ingredient : ingredients) {
    std::cout << ingredient.id << ". " << ingredient.name << std::endl;
  }
}

void ConsoleView::displayReviews(const std::vector<Review>& reviews,
                                  double averageRating) {
  std::cout << "=== ОТЗЫВЫ (Средний рейтинг: " << std::fixed
            << std::setprecision(1) << averageRating << ") ===" << std::endl;

  if (reviews.empty()) {
    displayMessage("Отзывы отсутствуют");
    return;
  }

  for (const auto& review : reviews) {
    std::cout << "Пользователь: " << review.username << " - "
              << "Оценка: " << review.rating << "/5" << std::endl;
    std::cout << "Дата: " << review.created_at << std::endl;
    if (!review.comment.empty()) {
      std::cout << "Комментарий: " << review.comment << std::endl;
    }
    std::cout << "-----------------------------------------------------------"
              << std::endl;
  }
}

void ConsoleView::displayUserProfile(const UserProfile& profile) {
  std::cout << "=== ПРОФИЛЬ ПОЛЬЗОВАТЕЛЯ ===" << std::endl;
  std::cout << "Имя пользователя: " << profile.username << std::endl;
  std::cout << "Email: " << profile.email << std::endl;
  std::cout << "Количество избранных рецептов: " << profile.favorites_count
            << std::endl;
  std::cout << "Количество отзывов: " << profile.reviews_count << std::endl;
  std::cout << "Количество созданных рецептов: " << profile.recipes_count
            << std::endl;
}

void ConsoleView::clearScreen() {
#ifdef _WIN32
  std::system("cls");
#else
  std::system("clear");
#endif
}

}  // namespace cli
