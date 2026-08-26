#ifndef IINGREDIENTREPOSITORY_H
#define IINGREDIENTREPOSITORY_H

#include "domain/entities/ingredient.h"
#include <QList>
#include <optional>
#include <utility>

class IIngredientRepository {
public:
    virtual ~IIngredientRepository() = default;

    // CRUD операции
    virtual std::optional<Ingredient> getIngredientById(int ingredientId) = 0;
    virtual QList<Ingredient> getAllIngredients() = 0;

    // Поиск ингредиентов по имени
    virtual QList<Ingredient> getIngredientsByName(const QString& name) = 0;

    // Операции с заменителями ингредиентов
    virtual QList<std::pair<int, double>> getIngredientSubstitutes(int ingredientId, double threshold = 0.7, int limit = 3) = 0;
};

#endif // IINGREDIENTREPOSITORY_H
