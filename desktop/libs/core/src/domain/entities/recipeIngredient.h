#ifndef RECIPEINGREDIENT_H
#define RECIPEINGREDIENT_H

#include <QString>

/**
 * @brief Класс, представляющий ингредиент в рецепте с информацией о количестве
 */
class RecipeIngredient
{
public:
    /**
     * @brief Конструктор по умолчанию
     */
    RecipeIngredient()
        : m_recipeId(-1)
        , m_ingredientId(-1)
        , m_quantity(0.0)
        , m_isOptional(false)
    {}

    /**
     * @brief Конструктор с параметрами
     * @param recipeId Идентификатор рецепта
     * @param ingredientId Идентификатор ингредиента
     * @param quantity Количество
     * @param unit Единица измерения
     * @param isOptional Признак опциональности ингредиента
     */
    RecipeIngredient(int recipeId,
                     int ingredientId,
                     double quantity,
                     const QString& unit,
                     bool isOptional = false)
        : m_recipeId(recipeId)
        , m_ingredientId(ingredientId)
        , m_quantity(quantity)
        , m_unit(unit)
        , m_isOptional(isOptional)
    {}

    // Геттеры
    int getRecipeId() const { return m_recipeId; }
    int getIngredientId() const { return m_ingredientId; }
    double getQuantity() const { return m_quantity; }
    QString getUnit() const { return m_unit; }
    bool isOptional() const { return m_isOptional; }

    // Сеттеры
    void setRecipeId(int recipeId) { m_recipeId = recipeId; }
    void setIngredientId(int ingredientId) { m_ingredientId = ingredientId; }
    void setQuantity(double quantity) { m_quantity = quantity; }
    void setUnit(const QString& unit) { m_unit = unit; }
    void setIsOptional(bool isOptional) { m_isOptional = isOptional; }

private:
    int m_recipeId;          ///< Идентификатор рецепта
    int m_ingredientId;      ///< Идентификатор ингредиента
    double m_quantity;       ///< Количество
    QString m_unit;          ///< Единица измерения
    bool m_isOptional;       ///< Признак опциональности ингредиента
};

#endif // RECIPEINGREDIENT_H
