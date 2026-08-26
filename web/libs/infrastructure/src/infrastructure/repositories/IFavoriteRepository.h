#ifndef IFAVORITEREPOSITORY_H
#define IFAVORITEREPOSITORY_H

#include <QList>
#include "domain/entities/favorite.h"

class IFavoriteRepository {
public:
    virtual ~IFavoriteRepository() = default;

    virtual QList<Favorite> getFavoritesByUserId(int userId) = 0;
    virtual bool addFavorite(const Favorite& favorite) = 0;
    virtual bool removeFavorite(int userId, int recipeId) = 0;
    virtual bool isFavorite(int userId, int recipeId) = 0;
};

#endif // IFAVORITEREPOSITORY_H
