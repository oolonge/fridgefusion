#ifndef ADMINRECIPEVIEW_H
#define ADMINRECIPEVIEW_H

#include <QList>
#include "domain/entities/recipe.h"
#include "domain/entities/category.h"
#include "domain/entities/ingredient.h"
#include "domain/dtos/recipeDetailDTO.h"
#include "domain/dtos/recipePreviewDTO.h"

// Структура для хранения данных формы редактирования рецепта
struct RecipeFormData {
    QString name;
    QString description;
    int preparationTime;
    int cookingTime;
    QString instructions;
};

class AdminRecipeView {
public:
    virtual ~AdminRecipeView() = default;

    // Отображение списка рецептов
    virtual void displayRecipes(const QList<RecipePreviewDTO>& recipes) = 0;
    
    // Отображение информации о рецепте
    virtual void displayRecipeDetails(const RecipeDetailDTO& recipeDetail) = 0;
    
    // Отображение категорий для выбора
    virtual void displayCategories(const QList<Category>& categories) = 0;
    
    // Отображение ингредиентов для выбора
    virtual void displayIngredients(const QList<Ingredient>& ingredients) = 0;
    
    // Уведомления о результатах операций
    virtual void showRecipeDeletedStatus(bool success, const QString& message) = 0;
    virtual void showRecipeUpdatedStatus(bool success, const QString& message) = 0;
    
    // Отображение ошибок
    virtual void showError(const QString& errorMessage) = 0;

    virtual void showMessage(const QString& message) = 0;
    
    // Навигация
    virtual void switchToMain() = 0;
    
    // Получение данных
    virtual int getSelectedRecipeId() const = 0;
    virtual RecipeFormData getRecipeFormData() const = 0;
    virtual QString getSelectedUnit() const = 0;

// signals:
//     virtual void createRecipeRequested(const QString& name) = 0;
};

#endif // ADMINRECIPEVIEW_H
