#include "favoriteProcessor.h"
#include <QDateTime>

FavoriteProcessor::FavoriteProcessor(IFavoriteRepository* favoriteRepository, std::shared_ptr<ILogger> logger)
    : favoriteRepository(favoriteRepository),
      logger(logger ? logger : LoggerService::getInstance().getLogger("default")) {
}

QList<Favorite> FavoriteProcessor::getUserFavorites(int userId) {
    return favoriteRepository->getFavoritesByUserId(userId);
}

bool FavoriteProcessor::addToFavorites(int userId, int recipeId) {
    try {
        Favorite favorite(0, userId, recipeId, QDateTime::currentDateTime());
        bool result = favoriteRepository->addFavorite(favorite);
        
        if (result) {
            logger->info(QString("Пользователь ID %1 добавил рецепт ID %2 в избранное").arg(userId).arg(recipeId));
        } else {
            logger->warn(QString("Не удалось добавить рецепт ID %1 в избранное для пользователя ID %2").arg(recipeId).arg(userId));
        }
        
        return result;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при добавлении рецепта в избранное: %1").arg(e.what()));
        return false;
    }
}

bool FavoriteProcessor::removeFromFavorites(int userId, int recipeId) {
    try {
        bool result = favoriteRepository->removeFavorite(userId, recipeId);
        
        if (result) {
            logger->info(QString("Пользователь ID %1 удалил рецепт ID %2 из избранного").arg(userId).arg(recipeId));
        } else {
            logger->warn(QString("Не удалось удалить рецепт ID %1 из избранного для пользователя ID %2").arg(recipeId).arg(userId));
        }
        
        return result;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при удалении рецепта из избранного: %1").arg(e.what()));
        return false;
    }
}

bool FavoriteProcessor::isRecipeFavorite(int userId, int recipeId) {
    return favoriteRepository->isFavorite(userId, recipeId);
}
