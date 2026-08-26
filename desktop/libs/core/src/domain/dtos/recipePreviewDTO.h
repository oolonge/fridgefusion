#ifndef RECIPEPREVIEWDTO_H
#define RECIPEPREVIEWDTO_H

#include <QString>
#include <QStringList>

class RecipePreviewDTO {
public:
    RecipePreviewDTO() = default;

    int id = 0;
    QString name;
    QString shortDescription;
    int totalTime = 0;         // Общее время приготовления в минутах
    QStringList mainIngredients; // 3-5 основных ингредиентов
    double averageRating = 0.0;  // Средний рейтинг
    bool isFavorite = false;     // В избранном ли у текущего пользователя
    QString authorName;          // Имя автора рецепта
};

#endif // RECIPEPREVIEWDTO_H
