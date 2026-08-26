#include "cli/recipe_handler.h"

#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

namespace cli {

RecipeHandler::RecipeHandler(std::shared_ptr<HttpClient> httpClient,
                              std::shared_ptr<ConfigManager> config,
                              std::shared_ptr<ConsoleView> view)
    : httpClient_(httpClient), config_(config), view_(view) {}

RecipeHandler::~RecipeHandler() {}

std::vector<RecipePreview> RecipeHandler::getAllRecipes(int limit, int offset) {
  std::string endpoint =
      "/recipes?limit=" + std::to_string(limit) + "&offset=" + std::to_string(offset);

  auto response = httpClient_->get(endpoint);

  if (response.success && response.statusCode == 200) {
    try {
      auto j = json::parse(response.body);
      std::vector<RecipePreview> recipes;

      // API returns array directly
      for (const auto& item : j) {
        RecipePreview recipe;
        recipe.id = item["id"].get<int>();
        recipe.name = item.value("name", "");
        recipe.description = item.value("description", "");
        recipe.total_time = item.value("total_time", 0);
        recipe.average_rating = item.value("average_rating", 0.0);
        recipes.push_back(recipe);
      }

      return recipes;
    } catch (const std::exception& e) {
      view_->displayError("Ошибка парсинга ответа сервера");
      if (config_->isVerbose()) {
        view_->displayError(e.what());
      }
    }
  } else {
    handleError(response);
  }

  return {};
}

RecipeDetail RecipeHandler::getRecipeDetails(int recipeId) {
  std::string endpoint = "/recipes/" + std::to_string(recipeId);
  std::string token = config_->getToken();

  auto response = httpClient_->get(endpoint, token);

  if (response.success && response.statusCode == 200) {
    try {
      auto j = json::parse(response.body);
      RecipeDetail recipe;

      recipe.id = j["id"].get<int>();
      recipe.name = j.value("name", "");
      recipe.description = j.value("description", "");
      recipe.preparation_time = j.value("preparation_time", 0);
      recipe.cooking_time = j.value("cooking_time", 0);
      recipe.average_rating = j.value("average_rating", 0.0);
      recipe.review_count = j.value("review_count", 0);

      // Инструкции
      if (j.contains("instructions")) {
        for (const auto& instruction : j["instructions"]) {
          recipe.instructions.push_back(instruction.get<std::string>());
        }
      }

      // Ингредиенты
      if (j.contains("ingredients")) {
        for (const auto& item : j["ingredients"]) {
          RecipeIngredient ingredient;
          ingredient.name = item.value("name", "");
          ingredient.quantity = item.value("quantity", 0.0);
          ingredient.unit = item.value("unit", "");
          ingredient.is_optional = item.value("is_optional", false);
          recipe.ingredients.push_back(ingredient);
        }
      }

      // Категории
      if (j.contains("categories")) {
        for (const auto& cat : j["categories"]) {
          // API возвращает категории как строки (названия), не числа
          if (cat.is_string()) {
            recipe.categories.push_back(cat.get<std::string>());
          } else if (cat.is_number()) {
            recipe.categories.push_back(std::to_string(cat.get<int>()));
          }
        }
      }

      return recipe;
    } catch (const std::exception& e) {
      view_->displayError("Ошибка парсинга ответа сервера");
      if (config_->isVerbose()) {
        view_->displayError(e.what());
      }
    }
  } else {
    handleError(response);
  }

  return RecipeDetail();
}

std::vector<RecipePreview> RecipeHandler::searchByIngredients(
    const std::vector<int>& ingredientIds) {
  if (ingredientIds.empty()) {
    return {};
  }

  // Формируем строку с ID ингредиентов
  std::ostringstream oss;
  for (size_t i = 0; i < ingredientIds.size(); ++i) {
    oss << ingredientIds[i];
    if (i < ingredientIds.size() - 1) {
      oss << ",";
    }
  }

  std::string endpoint = "/recipes?ingredients=" + oss.str();
  auto response = httpClient_->get(endpoint);

  if (response.success && response.statusCode == 200) {
    try {
      auto j = json::parse(response.body);
      std::vector<RecipePreview> recipes;

      if (j.contains("recipes")) {
        for (const auto& item : j["recipes"]) {
          RecipePreview recipe;
          recipe.id = item["id"].get<int>();
          recipe.name = item.value("name", "");
          recipe.description = item.value("short_description", "");
          recipe.total_time = item.value("total_time", 0);
          recipe.average_rating = item.value("average_rating", 0.0);
          recipes.push_back(recipe);
        }
      }

      return recipes;
    } catch (const std::exception& e) {
      view_->displayError("Ошибка парсинга ответа сервера");
      if (config_->isVerbose()) {
        view_->displayError(e.what());
      }
    }
  } else {
    handleError(response);
  }

  return {};
}

std::vector<RecipePreview> RecipeHandler::searchByCategory(int categoryId) {
  std::string endpoint = "/recipes?category_id=" + std::to_string(categoryId);
  auto response = httpClient_->get(endpoint);

  if (response.success && response.statusCode == 200) {
    try {
      auto j = json::parse(response.body);
      std::vector<RecipePreview> recipes;

      if (j.contains("recipes")) {
        for (const auto& item : j["recipes"]) {
          RecipePreview recipe;
          recipe.id = item["id"].get<int>();
          recipe.name = item.value("name", "");
          recipe.description = item.value("short_description", "");
          recipe.total_time = item.value("total_time", 0);
          recipe.average_rating = item.value("average_rating", 0.0);
          recipes.push_back(recipe);
        }
      }

      return recipes;
    } catch (const std::exception& e) {
      view_->displayError("Ошибка парсинга ответа сервера");
      if (config_->isVerbose()) {
        view_->displayError(e.what());
      }
    }
  } else {
    handleError(response);
  }

  return {};
}

std::vector<Ingredient> RecipeHandler::getAllIngredients() {
  auto response = httpClient_->get("/ingredients");

  if (response.success && response.statusCode == 200) {
    try {
      auto j = json::parse(response.body);
      std::vector<Ingredient> ingredients;

      for (const auto& item : j) {
        Ingredient ingredient;
        ingredient.id = item["id"].get<int>();
        ingredient.name = item.value("name", "");
        ingredients.push_back(ingredient);
      }

      return ingredients;
    } catch (const std::exception& e) {
      view_->displayError("Ошибка парсинга ответа сервера");
      if (config_->isVerbose()) {
        view_->displayError(e.what());
      }
    }
  } else {
    handleError(response);
  }

  return {};
}

std::vector<Category> RecipeHandler::getAllCategories() {
  auto response = httpClient_->get("/categories");

  if (response.success && response.statusCode == 200) {
    try {
      auto j = json::parse(response.body);
      std::vector<Category> categories;

      for (const auto& item : j) {
        Category category;
        category.id = item["id"].get<int>();
        category.name = item.value("name", "");
        category.description = item.value("description", "");
        categories.push_back(category);
      }

      return categories;
    } catch (const std::exception& e) {
      view_->displayError("Ошибка парсинга ответа сервера");
      if (config_->isVerbose()) {
        view_->displayError(e.what());
      }
    }
  } else {
    handleError(response);
  }

  return {};
}

std::vector<Review> RecipeHandler::getRecipeReviews(int recipeId) {
  std::string endpoint = "/recipes/" + std::to_string(recipeId) + "/reviews";
  auto response = httpClient_->get(endpoint);

  if (response.success && response.statusCode == 200) {
    try {
      auto j = json::parse(response.body);
      std::vector<Review> reviews;

      if (j.contains("reviews")) {
        for (const auto& item : j["reviews"]) {
          Review review;
          review.id = item["id"].get<int>();
          review.username = item.value("username", "");
          review.rating = item.value("rating", 0);
          review.comment = item.value("comment", "");
          review.created_at = item.value("date_posted", "");
          reviews.push_back(review);
        }
      }

      return reviews;
    } catch (const std::exception& e) {
      view_->displayError("Ошибка парсинга ответа сервера");
      if (config_->isVerbose()) {
        view_->displayError(e.what());
      }
    }
  } else {
    handleError(response);
  }

  return {};
}

void RecipeHandler::handleError(const HttpResponse& response) {
  if (config_->isVerbose()) {
    view_->displayError("HTTP " + std::to_string(response.statusCode) + ": " +
                        response.body);
    return;
  }

  try {
    auto j = json::parse(response.body);
    if (j.contains("message")) {
      view_->displayError(j["message"].get<std::string>());
    } else {
      view_->displayError("Ошибка сервера");
    }
  } catch (const std::exception&) {
    view_->displayError("Ошибка соединения с сервером");
  }
}

}  // namespace cli
