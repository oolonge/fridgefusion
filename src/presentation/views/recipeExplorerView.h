#ifndef RECIPEEXPLORERVIEW_H
#define RECIPEEXPLORERVIEW_H

#include <QList>
#include <QString>
#include "domain/dtos/recipePreviewDTO.h"
#include "domain/entities/category.h"
#include "domain/entities/ingredient.h"

class RecipeExplorerView {
public:
    virtual ~RecipeExplorerView() = default;

    // Методы для обновления представления
    virtual void displayRecipes(const QList<RecipePreviewDTO>& recipes) = 0;
    virtual void displayCategories(const QList<Category>& categories) = 0;
    virtual void displayIngredients(const QList<Ingredient>& ingredients) = 0;
    virtual void showNoRecipesFound() = 0;
    virtual void displaySearchResults(const QList<RecipePreviewDTO>& recipes) = 0;

    // Методы для получения данных фильтрации и поиска
    virtual QList<int> getSelectedIngredientIds() = 0;
    virtual QList<int> getSelectedCategoryIds() = 0;
    virtual int getMaxCookingTime() = 0;

    // Навигация
    virtual void showRecipeDetails(int recipeId) = 0;
};

#endif // RECIPEEXPLORERVIEW_H
