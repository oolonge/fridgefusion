#include "cli/auth_handler.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace cli {

AuthHandler::AuthHandler(std::shared_ptr<HttpClient> httpClient,
                         std::shared_ptr<ConfigManager> config,
                         std::shared_ptr<ConsoleView> view)
    : httpClient_(httpClient), config_(config), view_(view) {}

AuthHandler::~AuthHandler() {}

bool AuthHandler::login(const std::string& email, const std::string& password) {
  json body = {{"email", email}, {"password", password}};

  auto response = httpClient_->post("/auth/login", body.dump());

  if (response.success && response.statusCode == 200) {
    try {
      auto j = json::parse(response.body);
      std::string token = j["access_token"].get<std::string>();
      int userId = j["user"]["id"].get<int>();

      config_->setToken(token);
      config_->setUserId(userId);
      config_->save();

      view_->displaySuccess("Вход выполнен успешно");
      return true;
    } catch (const std::exception& e) {
      view_->displayError("Ошибка парсинга ответа сервера");
      if (config_->isVerbose()) {
        view_->displayError(e.what());
      }
      return false;
    }
  }

  handleError(response);
  return false;
}

bool AuthHandler::registerUser(const std::string& username,
                                const std::string& email,
                                const std::string& password) {
  json body = {{"username", username}, {"email", email}, {"password", password}};

  auto response = httpClient_->post("/auth/register", body.dump());

  if (response.success && response.statusCode == 201) {
    try {
      auto j = json::parse(response.body);
      std::string token = j["access_token"].get<std::string>();
      int userId = j["user"]["id"].get<int>();

      config_->setToken(token);
      config_->setUserId(userId);
      config_->save();

      view_->displaySuccess("Регистрация выполнена успешно");
      return true;
    } catch (const std::exception& e) {
      view_->displayError("Ошибка парсинга ответа сервера");
      if (config_->isVerbose()) {
        view_->displayError(e.what());
      }
      return false;
    }
  }

  handleError(response);
  return false;
}

bool AuthHandler::logout() {
  std::string token = config_->getToken();
  if (token.empty()) {
    return true;
  }

  auto response = httpClient_->post("/auth/logout", "{}", token);

  // Очищаем токен независимо от ответа сервера
  config_->setToken("");
  config_->setUserId(-1);
  config_->save();

  if (response.success && response.statusCode == 200) {
    view_->displaySuccess("Вы вышли из системы");
    return true;
  }

  // Не показываем ошибку, если токен уже недействителен
  if (response.statusCode == 401) {
    return true;
  }

  handleError(response);
  return false;
}

bool AuthHandler::isLoggedIn() const {
  return !config_->getToken().empty();
}

void AuthHandler::handleError(const HttpResponse& response) {
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
