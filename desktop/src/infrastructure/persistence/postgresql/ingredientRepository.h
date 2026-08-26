#ifndef INGREDIENTREPOSITORY_H
#define INGREDIENTREPOSITORY_H

#include "infrastructure/repositories/IIngredientRepository.h"
#include "infrastructure/persistence/database/dbContext.h"
#include <memory>

class IngredientRepository : public IIngredientRepository {
public:
    explicit IngredientRepository(std::shared_ptr<DbContext> dbContext);
    ~IngredientRepository() override = default;

    // Основные операции с ингредиентами
    std::optional<Ingredient> getIngredientById(int ingredientId) override;
    QList<Ingredient> getAllIngredients() override;
    QList<Ingredient> getIngredientsByName(const QString& name) override;

    // Получение заменителей ингредиентов
    QList<std::pair<int, double>> getIngredientSubstitutes(int ingredientId, double threshold, int limit) override;

private:
    std::shared_ptr<DbContext> dbContext;

    // Вспомогательный метод для преобразования записи БД в объект
    Ingredient mapToIngredient(const QSqlRecord& record);
};

#endif // INGREDIENTREPOSITORY_H
