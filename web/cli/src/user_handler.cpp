#include "cli/user_handler.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace cli {

UserHandler::UserHandler(std::shared_ptr<HttpClient> httpClient,
                         std::shared_ptr<ConfigManager> config,
                         std::shared_ptr<ConsoleView> view)
    : httpClient_(httpClient), config_(config), view_(view) {}

UserHandler::~UserHandler() {}

UserProfile UserHandler::getProfile() {
  std::string token = config_->getToken();
  if (token.empty()) {
    view_->displayError("Необходима авторизация");
    return UserProfile();
  }

  auto response = httpClient_->get("/users/me", token);

  if (response.success && response.statusCode == 200) {
    try {
      auto j = json::parse(response.body);
      UserProfile profile;

      profile.id = j["id"].get<int>();
      profile.username = j.value("username", "");
      profile.email = j.value("email", "");
      profile.favorites_count = j.value("favorites_count", 0);
      profile.reviews_count = j.value("reviews_count", 0);
      profile.recipes_count = j.value("recipes_count", 0);

      return profile;
    } catch (const std::exception& e) {
      view_->displayError("Ошибка парсинга ответа сервера");
      if (config_->isVerbose()) {
        view_->displayError(e.what());
      }
    }
  } else {
    handleError(response);
  }

  return UserProfile();
}

void UserHandler::handleError(const HttpResponse& response) {
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
