#ifndef AUTH_HANDLER_H
#define AUTH_HANDLER_H

#include <memory>
#include <string>

#include "cli/config_manager.h"
#include "cli/console_view.h"
#include "cli/http_client.h"

namespace cli {

// Обработчик авторизации
class AuthHandler {
 public:
  AuthHandler(std::shared_ptr<HttpClient> httpClient,
              std::shared_ptr<ConfigManager> config,
              std::shared_ptr<ConsoleView> view);
  ~AuthHandler();

  // Методы авторизации
  bool login(const std::string& email, const std::string& password);
  bool registerUser(const std::string& username, const std::string& email,
                    const std::string& password);
  bool logout();
  bool isLoggedIn() const;

 private:
  std::shared_ptr<HttpClient> httpClient_;
  std::shared_ptr<ConfigManager> config_;
  std::shared_ptr<ConsoleView> view_;

  // Обработка ошибок API
  void handleError(const HttpResponse& response);
};

}  // namespace cli

#endif  // AUTH_HANDLER_H
