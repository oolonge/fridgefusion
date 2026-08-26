#include "recipeFilterService.h"

QList<Recipe> RecipeFilterService::filterRecipes(const QList<Recipe>& recipes, const RecipeFilterDTO& filterDTO) {
    QList<Recipe> filteredRecipes = recipes;

    // Применяем фильтр по времени приготовления
    if (filterDTO.getMaxPrepTime() > 0 || filterDTO.getMaxCookingTime() > 0) {
        filteredRecipes = filterByTime(filteredRecipes, filterDTO.getMaxPrepTime(), filterDTO.getMaxCookingTime());
    }

    // Применяем фильтр по категориям
    if (!filterDTO.getCategoryIds().isEmpty()) {
        filteredRecipes = filterByCategories(filteredRecipes, filterDTO.getCategoryIds());
    }

    return filteredRecipes;
}

QList<Recipe> RecipeFilterService::filterByTime(const QList<Recipe>& recipes, int maxPrepTime, int maxCookTime) {
    QList<Recipe> filteredRecipes;

    for (const Recipe& recipe : recipes) {
        bool matchesPrepTime = (maxPrepTime <= 0 || recipe.getPreparationTime() <= maxPrepTime);
        bool matchesCookTime = (maxCookTime <= 0 || recipe.getCookingTime() <= maxCookTime);

        if (matchesPrepTime && matchesCookTime) {
            filteredRecipes.append(recipe);
        }
    }

    return filteredRecipes;
}

QList<Recipe> RecipeFilterService::filterByCategories(const QList<Recipe>& recipes, const QList<int>& categoryIds) {
    QList<Recipe> filteredRecipes;

    for (const Recipe& recipe : recipes) {
        if (matchesCategoryFilter(recipe, categoryIds)) {
            filteredRecipes.append(recipe);
        }
    }

    return filteredRecipes;
}

bool RecipeFilterService::matchesCategoryFilter(const Recipe& recipe, const QList<int>& categoryIds) {
    const QList<int>& recipeCategoryIds = recipe.getCategoryIds();

    // Проверяем, есть ли хотя бы одна общая категория
    for (int categoryId : categoryIds) {
        if (recipeCategoryIds.contains(categoryId)) {
            return true;
        }
    }

    return false;
}
