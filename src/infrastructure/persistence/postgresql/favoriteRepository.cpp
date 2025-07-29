#include "favoriteRepository.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <stdexcept>

FavoriteRepository::FavoriteRepository(DbContext* dbContext)
    : dbContext(dbContext) {
}

QList<Favorite> FavoriteRepository::getFavoritesByUserId(int userId) {
    QList<Favorite> favorites;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["user_id"] = userId;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT * FROM favorites WHERE user_id = :user_id ORDER BY date_added DESC", params);

        while (query.next()) {
            favorites.append(mapToFavorite(query.record()));
        }
        return favorites;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении избранных рецептов пользователя:" << e.what();
        throw;
    }
}

bool FavoriteRepository::addFavorite(const Favorite& favorite) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Проверка на наличие дубликата
        if (isFavorite(favorite.getUserId(), favorite.getRecipeId())) {
            return true; // Уже существует, считаем операцию успешной
        }

        QVariantMap favoriteData;
        favoriteData["user_id"] = favorite.getUserId();
        favoriteData["recipe_id"] = favorite.getRecipeId();
        favoriteData["date_added"] = favorite.getDateAdded();

        return dbContext->insert("favorites", favoriteData);
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при добавлении рецепта в избранное:" << e.what();
        throw;
    }
}

bool FavoriteRepository::removeFavorite(int userId, int recipeId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QString whereClause = QString("user_id = %1 AND recipe_id = %2").arg(userId).arg(recipeId);
        return dbContext->remove("favorites", whereClause);
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при удалении рецепта из избранного:" << e.what();
        throw;
    }
}

bool FavoriteRepository::isFavorite(int userId, int recipeId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["user_id"] = userId;
        params["recipe_id"] = recipeId;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT COUNT(*) FROM favorites WHERE user_id = :user_id AND recipe_id = :recipe_id", params);

        if (query.next()) {
            return query.value(0).toInt() > 0;
        }
        return false;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при проверке наличия рецепта в избранном:" << e.what();
        throw;
    }
}

Favorite FavoriteRepository::mapToFavorite(const QSqlRecord& record) {
    int id = record.value("id").toInt();
    int userId = record.value("user_id").toInt();
    int recipeId = record.value("recipe_id").toInt();
    QDateTime dateAdded = record.value("date_added").toDateTime();

    return Favorite(id, userId, recipeId, dateAdded);
}
