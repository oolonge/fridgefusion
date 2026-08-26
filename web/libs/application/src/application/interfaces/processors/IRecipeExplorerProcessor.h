#ifndef IRECIPEEXPLORERPROCESSOR_H
#define IRECIPEEXPLORERPROCESSOR_H

#include <QList>
#include <QString>
#include <optional>
#include "domain/entities/recipe.h"
#include "domain/entities/ingredient.h"
#include "domain/dtos/recipeDetailDTO.h"
#include "domain/dtos/recipePreviewDTO.h"
#include "domain/entities/category.h"

class IRecipeExplorerProcessor {
public:
    virtual ~IRecipeExplorerProcessor() = default;

    // Получение превью всех рецептов
    virtual QList<RecipePreviewDTO> getAllRecipePreviews(int limit = 100, int offset = 0) = 0;

    // Получение всех категорий
    virtual QList<Category> getAllCategories() = 0;

    // Добавляем метод для получения всех ингредиентов
    virtual QList<Ingredient> getAllIngredients() = 0;

    // Поиск рецептов по ингредиентам (с учетом возможных замен)
    virtual QList<Recipe> findRecipesWithIngredients(const QList<int>& ingredientIds) = 0;

    // Поиск рецептов по категории
    virtual QList<Recipe> findRecipesByCategory(int categoryId) = 0;

    // Поиск рецептов по категории и ингредиентам
    virtual QList<Recipe> findRecipesByCategoryAndIngredients(int categoryId, const QList<int>& ingredientIds) = 0;

    // Получение возможных заменителей для ингредиента
    virtual QList<std::pair<int, double>> getIngredientSubstitutes(int ingredientId, int limit = 3) = 0;

    // Получение базовой информации о рецепте для предпросмотра
    virtual RecipePreviewDTO getRecipePreview(int recipeId) = 0;

    // Получение полной информации о рецепте
    virtual RecipeDetailDTO getRecipeDetail(int recipeId) = 0;
};

#endif // IRECIPEEXPLORERPROCESSOR_H
