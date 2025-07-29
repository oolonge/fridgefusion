#ifndef FAVORITEPROCESSOR_H
#define FAVORITEPROCESSOR_H

#include "application/interfaces/processors/IFavoriteProcessor.h"
#include "infrastructure/repositories/IFavoriteRepository.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerService.h"

class FavoriteProcessor : public IFavoriteProcessor {
public:
    explicit FavoriteProcessor(IFavoriteRepository* favoriteRepository, std::shared_ptr<ILogger> logger = nullptr);
    ~FavoriteProcessor() override = default;

    QList<Favorite> getUserFavorites(int userId) override;
    bool addToFavorites(int userId, int recipeId) override;
    bool removeFromFavorites(int userId, int recipeId) override;
    bool isRecipeFavorite(int userId, int recipeId) override;

private:
    IFavoriteRepository* favoriteRepository;
    std::shared_ptr<ILogger> logger;
};

#endif // FAVORITEPROCESSOR_H
