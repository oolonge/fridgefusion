#ifndef FAVORITE_HANDLER_H
#define FAVORITE_HANDLER_H

#include <memory>
#include <vector>

#include "cli/config_manager.h"
#include "cli/console_view.h"
#include "cli/http_client.h"

namespace cli {

// Обработчик избранного
class FavoriteHandler {
 public:
  FavoriteHandler(std::shared_ptr<HttpClient> httpClient,
                  std::shared_ptr<ConfigManager> config,
                  std::shared_ptr<ConsoleView> view);
  ~FavoriteHandler();

  // Управление избранным
  std::vector<RecipePreview> getFavorites();
  bool addFavorite(int recipeId);
  bool removeFavorite(int recipeId);
  bool isFavorite(int recipeId);

 private:
  std::shared_ptr<HttpClient> httpClient_;
  std::shared_ptr<ConfigManager> config_;
  std::shared_ptr<ConsoleView> view_;

  void handleError(const HttpResponse& response);
};

}  // namespace cli

#endif  // FAVORITE_HANDLER_H
