#ifndef USER_HANDLER_H
#define USER_HANDLER_H

#include <memory>

#include "cli/config_manager.h"
#include "cli/console_view.h"
#include "cli/http_client.h"

namespace cli {

// Обработчик профиля пользователя
class UserHandler {
 public:
  UserHandler(std::shared_ptr<HttpClient> httpClient,
              std::shared_ptr<ConfigManager> config,
              std::shared_ptr<ConsoleView> view);
  ~UserHandler();

  // Получение профиля
  UserProfile getProfile();

 private:
  std::shared_ptr<HttpClient> httpClient_;
  std::shared_ptr<ConfigManager> config_;
  std::shared_ptr<ConsoleView> view_;

  void handleError(const HttpResponse& response);
};

}  // namespace cli

#endif  // USER_HANDLER_H
