#ifndef RECIPEDETAILDTO_H
#define RECIPEDETAILDTO_H

#include <QString>
#include <QStringList>
#include <QList>
#include <utility>

class RecipeDetailDTO {
public:
    RecipeDetailDTO() = default;

    int id = 0;
    QString name;
    QString fullDescription;
    int preparationTime = 0;
    int cookingTime = 0;
    QList<QString> categories;

    struct IngredientInfo {
        int id;
        QString name;
        double quantity;
        QString unit;
        bool isOptional;
        QList<std::pair<int, QString>> possibleSubstitutes; // id и название заменителей
    };

    QList<IngredientInfo> ingredients;
    QStringList steps;

    double averageRating = 0.0;
    int reviewCount = 0;
    bool isFavorite = false;
};

#endif // RECIPEDETAILDTO_H
