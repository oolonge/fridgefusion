#ifndef RECIPE_H
#define RECIPE_H

#include <QString>
#include <QStringList>
#include <QList>
#include "recipeIngredient.h"

/**
 * @brief Класс, представляющий рецепт
 */
class Recipe
{
public:
    /**
     * @brief Конструктор по умолчанию
     */
    Recipe()
        : m_recipeId(-1)
        , m_preparationTime(0)
        , m_cookingTime(0)
    {}

    /**
     * @brief Конструктор с параметрами
     * @param recipeId Идентификатор рецепта
     * @param name Название рецепта
     * @param description Описание рецепта
     * @param preparationTime Время подготовки в минутах
     * @param cookingTime Время приготовления в минутах
     * @param categoryIds Список идентификаторов категорий рецепта
     * @param steps Шаги приготовления
     * @param ingredients Список ингредиентов с количеством
     */
    Recipe(int recipeId,
           const QString& name,
           const QString& description,
           int preparationTime,
           int cookingTime,
           const QList<int>& categoryIds,
           const QStringList& steps,
           const QList<RecipeIngredient>& ingredients = QList<RecipeIngredient>())
        : m_recipeId(recipeId)
        , m_name(name)
        , m_description(description)
        , m_preparationTime(preparationTime)
        , m_cookingTime(cookingTime)
        , m_categoryIds(categoryIds)
        , m_steps(steps)
        , m_ingredients(ingredients)
    {}

    // Геттеры
    int getRecipeId() const { return m_recipeId; }
    QString getName() const { return m_name; }
    QString getDescription() const { return m_description; }
    int getPreparationTime() const { return m_preparationTime; }
    int getCookingTime() const { return m_cookingTime; }
    QList<int> getCategoryIds() const { return m_categoryIds; }
    QStringList getSteps() const { return m_steps; }
    QList<RecipeIngredient> getIngredients() const { return m_ingredients; }

    // Сеттеры
    void setRecipeId(int recipeId) { m_recipeId = recipeId; }
    void setName(const QString& name) { m_name = name; }
    void setDescription(const QString& description) { m_description = description; }
    void setPreparationTime(int preparationTime) { m_preparationTime = preparationTime; }
    void setCookingTime(int cookingTime) { m_cookingTime = cookingTime; }
    void setCategoryIds(const QList<int>& categoryIds) { m_categoryIds = categoryIds; }
    void setSteps(const QStringList& steps) { m_steps = steps; }
    void setIngredients(const QList<RecipeIngredient>& ingredients) { m_ingredients = ingredients; }

    // Управление категориями
    void addCategoryId(int categoryId) { m_categoryIds.append(categoryId); }
    void removeCategoryId(int categoryId) { m_categoryIds.removeAll(categoryId); }
    bool hasCategory(int categoryId) const { return m_categoryIds.contains(categoryId); }

    // Управление ингредиентами
    void addIngredient(const RecipeIngredient& ingredient) { m_ingredients.append(ingredient); }
    void removeIngredient(int ingredientId) {
        for (int i = 0; i < m_ingredients.size(); ++i) {
            if (m_ingredients[i].getIngredientId() == ingredientId) {
                m_ingredients.removeAt(i);
                break;
            }
        }
    }

    // Управление шагами
    void addStep(const QString& step) { m_steps.append(step); }
    void removeStep(int index) {
        if (index >= 0 && index < m_steps.size()) {
            m_steps.removeAt(index);
        }
    }

    /**
     * @brief Получить общее время приготовления (подготовка + готовка)
     * @return Общее время в минутах
     */
    int getTotalTime() const { return m_preparationTime + m_cookingTime; }

private:
    int m_recipeId;                        ///< Идентификатор рецепта
    QString m_name;                        ///< Название рецепта
    QString m_description;                 ///< Описание рецепта
    int m_preparationTime;                 ///< Время подготовки в минутах
    int m_cookingTime;                     ///< Время приготовления в минутах
    QList<int> m_categoryIds;              ///< Список идентификаторов категорий
    QStringList m_steps;                   ///< Шаги приготовления
    QList<RecipeIngredient> m_ingredients; ///< Список ингредиентов с количеством
};

#endif // RECIPE_H
