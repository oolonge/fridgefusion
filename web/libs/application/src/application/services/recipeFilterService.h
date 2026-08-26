#ifndef RECIPEFILTERSERVICE_H
#define RECIPEFILTERSERVICE_H

#include "application/interfaces/services/IRecipeFilterService.h"
#include "domain/entities/recipe.h"
#include "domain/dtos/recipeFilterDTO.h"
#include <QList>

class RecipeFilterService : public IRecipeFilterService {
public:
    RecipeFilterService() = default;
    ~RecipeFilterService() override = default;

    QList<Recipe> filterRecipes(const QList<Recipe>& recipes, const RecipeFilterDTO& filterDTO) override;
    QList<Recipe> filterByTime(const QList<Recipe>& recipes, int maxPrepTime = -1, int maxCookTime = -1) override;
    QList<Recipe> filterByCategories(const QList<Recipe>& recipes, const QList<int>& categoryIds) override;

private:
    bool matchesCategoryFilter(const Recipe& recipe, const QList<int>& categoryIds);
};

#endif // RECIPEFILTERSERVICE_H
