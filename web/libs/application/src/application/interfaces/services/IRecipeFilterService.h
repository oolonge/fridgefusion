#ifndef IRECIPEFILTERSERVICE_H
#define IRECIPEFILTERSERVICE_H

#include <QList>
#include "domain/entities/recipe.h"
#include "domain/dtos/recipeFilterDTO.h"

class IRecipeFilterService {
public:
    virtual ~IRecipeFilterService() = default;

    // Фильтрует список рецептов по заданным критериям
    virtual QList<Recipe> filterRecipes(const QList<Recipe>& recipes, const RecipeFilterDTO& filterDTO) = 0;

    // Фильтрует рецепты только по времени приготовления
    virtual QList<Recipe> filterByTime(const QList<Recipe>& recipes, int maxPrepTime = -1, int maxCookTime = -1) = 0;

    // Фильтрует рецепты только по категориям
    virtual QList<Recipe> filterByCategories(const QList<Recipe>& recipes, const QList<int>& categoryIds) = 0;
};

#endif // IRECIPEFILTERSERVICE_H
