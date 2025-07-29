#ifndef FAVORITEREPOSITORY_H
#define FAVORITEREPOSITORY_H

#include "infrastructure/repositories/IFavoriteRepository.h"
#include "infrastructure/persistence/database/dbContext.h"

class FavoriteRepository : public IFavoriteRepository {
public:
    explicit FavoriteRepository(DbContext* dbContext);
    ~FavoriteRepository() override = default;

    QList<Favorite> getFavoritesByUserId(int userId) override;
    bool addFavorite(const Favorite& favorite) override;
    bool removeFavorite(int userId, int recipeId) override;
    bool isFavorite(int userId, int recipeId) override;

private:
    DbContext* dbContext;

    // Вспомогательный метод для преобразования записи БД в объект
    Favorite mapToFavorite(const QSqlRecord& record);
};

#endif // FAVORITEREPOSITORY_H
