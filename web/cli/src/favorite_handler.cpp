#include "cli/favorite_handler.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace cli {

FavoriteHandler::FavoriteHandler(std::shared_ptr<HttpClient> httpClient,
                                 std::shared_ptr<ConfigManager> config,
                                 std::shared_ptr<ConsoleView> view)
    : httpClient_(httpClient), config_(config), view_(view) {}

FavoriteHandler::~FavoriteHandler() {}

std::vector<RecipePreview> FavoriteHandler::getFavorites() {
  std::string token = config_->getToken();
  if (token.empty()) {
    view_->displayError("Необходима авторизация");
    return {};
  }

  auto response = httpClient_->get("/favorites", token);

  if (response.success && response.statusCode == 200) {
    try {
      auto j = json::parse(response.body);
      std::vector<RecipePreview> recipes;

      for (const auto& item : j) {
        RecipePreview recipe;
        recipe.id = item["recipe_id"].get<int>();
        recipe.name = item.value("recipe_name", "");
        recipe.description = "";
        recipe.total_time = 0;
        recipe.average_rating = 0.0;
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

bool FavoriteHandler::addFavorite(int recipeId) {
  std::string token = config_->getToken();
  if (token.empty()) {
    view_->displayError("Необходима авторизация");
    return false;
  }

  json body = {{"recipe_id", recipeId}};

  auto response = httpClient_->post("/favorites", body.dump(), token);

  if (response.success && response.statusCode == 201) {
    view_->displaySuccess("Рецепт добавлен в избранное");
    return true;
  }

  // Не показываем ошибку, если уже в избранном
  if (response.statusCode == 409) {
    view_->displayMessage("Рецепт уже в избранном");
    return false;
  }

  handleError(response);
  return false;
}

bool FavoriteHandler::removeFavorite(int recipeId) {
  std::string token = config_->getToken();
  if (token.empty()) {
    view_->displayError("Необходима авторизация");
    return false;
  }

  std::string endpoint = "/favorites/" + std::to_string(recipeId);
  auto response = httpClient_->deleteRequest(endpoint, token);

  if (response.success && response.statusCode == 204) {
    view_->displaySuccess("Рецепт удален из избранного");
    return true;
  }

  handleError(response);
  return false;
}

bool FavoriteHandler::isFavorite(int recipeId) {
  auto favorites = getFavorites();
  for (const auto& recipe : favorites) {
    if (recipe.id == recipeId) {
      return true;
    }
  }
  return false;
}

void FavoriteHandler::handleError(const HttpResponse& response) {
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
