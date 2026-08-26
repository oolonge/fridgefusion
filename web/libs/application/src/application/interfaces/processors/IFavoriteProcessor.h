#ifndef IFAVORITEPROCESSOR_H
#define IFAVORITEPROCESSOR_H

#include <QList>
#include "domain/entities/favorite.h"

class IFavoriteProcessor {
public:
    virtual ~IFavoriteProcessor() = default;

    virtual QList<Favorite> getUserFavorites(int userId) = 0;
    virtual bool addToFavorites(int userId, int recipeId) = 0;
    virtual bool removeFromFavorites(int userId, int recipeId) = 0;
    virtual bool isRecipeFavorite(int userId, int recipeId) = 0;
};

#endif // IFAVORITEPROCESSOR_H
