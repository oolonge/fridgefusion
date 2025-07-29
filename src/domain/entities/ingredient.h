#ifndef INGREDIENT_H
#define INGREDIENT_H

#include <QString>

/**
 * @brief Класс, представляющий ингредиент
 */
class Ingredient
{
public:
    /**
     * @brief Конструктор по умолчанию
     */
    Ingredient() : m_ingredientId(-1), m_calories(0.0), m_defaultExpirationDays(0) {}

    /**
     * @brief Конструктор с параметрами
     * @param ingredientId Идентификатор ингредиента
     * @param name Название ингредиента
     * @param defaultUnit Единица измерения по умолчанию
     * @param calories Калорийность на 100г/мл
     * @param defaultExpirationDays Срок годности в днях по умолчанию
     */
    Ingredient(int ingredientId,
               const QString& name,
               const QString& defaultUnit,
               double calories,
               int defaultExpirationDays)
        : m_ingredientId(ingredientId)
        , m_name(name)
        , m_defaultUnit(defaultUnit)
        , m_calories(calories)
        , m_defaultExpirationDays(defaultExpirationDays)
    {}

    // Геттеры
    int getIngredientId() const { return m_ingredientId; }
    QString getName() const { return m_name; }
    QString getDefaultUnit() const { return m_defaultUnit; }
    double getCalories() const { return m_calories; }
    int getDefaultExpirationDays() const { return m_defaultExpirationDays; }

    // Сеттеры
    void setIngredientId(int ingredientId) { m_ingredientId = ingredientId; }
    void setName(const QString& name) { m_name = name; }
    void setDefaultUnit(const QString& defaultUnit) { m_defaultUnit = defaultUnit; }
    void setCalories(double calories) { m_calories = calories; }
    void setDefaultExpirationDays(int defaultExpirationDays) { m_defaultExpirationDays = defaultExpirationDays; }

private:
    int m_ingredientId;               ///< Идентификатор ингредиента
    QString m_name;                   ///< Название ингредиента
    QString m_defaultUnit;            ///< Единица измерения по умолчанию
    double m_calories;                ///< Калорийность на 100г/мл
    int m_defaultExpirationDays;      ///< Срок годности в днях по умолчанию
};

#endif // INGREDIENT_H
