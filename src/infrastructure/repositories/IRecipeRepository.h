#ifndef IRECIPEREPOSITORY_H
#define IRECIPEREPOSITORY_H

#include <QList>
#include <QString>
#include <optional>
#include "domain/entities/recipe.h"
#include "domain/entities/recipeIngredient.h"
#include "domain/entities/category.h"
#include "domain/dtos/recipePreviewDTO.h"
#include "domain/dtos/recipeDetailDTO.h"

class IRecipeRepository {
public:
    virtual ~IRecipeRepository() = default;

    // Базовые CRUD операции
    virtual std::optional<Recipe> getRecipeById(int recipeId) = 0;
    virtual QList<Recipe> getAllRecipes(int limit = 100, int offset = 0) = 0;
    virtual bool updateRecipe(int recipeId, const QString& name, const QString& description, 
                            int preparationTime, int cookingTime, const QString& instructions) = 0;
    virtual bool deleteRecipe(int recipeId) = 0;

    // Категории рецептов
    virtual QList<Category> getAllCategories() = 0;
    virtual bool addCategoryToRecipe(int recipeId, int categoryId) = 0;
    virtual bool removeCategoryFromRecipe(int recipeId, int categoryId) = 0;

    // Ингредиенты рецептов
    virtual QList<RecipeIngredient> getRecipeIngredients(int recipeId) = 0;
    virtual bool addIngredientToRecipe(int recipeId, int ingredientId, double quantity, 
                                     const QString& unit, bool isOptional = false) = 0;
    virtual bool removeIngredientFromRecipe(int recipeId, int ingredientId) = 0;
    virtual bool updateIngredientQuantity(int recipeId, int ingredientId, double quantity) = 0;

    // Поиск рецептов
    virtual QList<Recipe> getRecipesByIngredients(const QList<int>& ingredientIds) = 0;
    virtual QList<Recipe> getRecipesByCategory(int categoryId) = 0;

    // Поиск заменителей ингредиентов
    virtual QList<std::pair<int, double>> getIngredientSubstitutes(int ingredientId, int limit = 3) = 0;

    // Получение подробной информации для DTO
    virtual RecipePreviewDTO getRecipePreview(int recipeId) = 0;
    virtual RecipeDetailDTO getRecipeDetail(int recipeId) = 0;

    virtual int createRecipe(const QString& name, const QString& description,
                             int preparationTime, int cookingTime,
                             const QString& instructions, int authorId) = 0;
};

#endif // IRECIPEREPOSITORY_H
