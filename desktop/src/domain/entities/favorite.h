#ifndef FAVORITE_H
#define FAVORITE_H

#include <QDateTime>

class Favorite {
public:
    Favorite() = default;
    Favorite(int id, int userId, int recipeId, const QDateTime& dateAdded)
        : id(id), userId(userId), recipeId(recipeId), dateAdded(dateAdded) {}

    int getId() const { return id; }
    int getUserId() const { return userId; }
    int getRecipeId() const { return recipeId; }
    QDateTime getDateAdded() const { return dateAdded; }

private:
    int id = 0;
    int userId = 0;
    int recipeId = 0;
    QDateTime dateAdded;
};

#endif // FAVORITE_H
