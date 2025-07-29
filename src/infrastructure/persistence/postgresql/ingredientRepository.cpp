#include "ingredientRepository.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <stdexcept>

IngredientRepository::IngredientRepository(std::shared_ptr<DbContext> dbContext)
    : dbContext(std::move(dbContext)) {
}

std::optional<Ingredient> IngredientRepository::getIngredientById(int ingredientId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["id"] = ingredientId;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT i.*, u.name AS unit_name FROM ingredients i "
            "LEFT JOIN units u ON i.default_unit_id = u.id "
            "WHERE i.id = :id", params);

        if (query.next()) {
            return mapToIngredient(query.record());
        }
        return std::nullopt;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении ингредиента по ID:" << e.what();
        throw;
    }
}

QList<Ingredient> IngredientRepository::getAllIngredients() {
    QList<Ingredient> ingredients;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QSqlQuery query = dbContext->executeQuery(
            "SELECT i.*, u.name AS unit_name FROM ingredients i "
            "LEFT JOIN units u ON i.default_unit_id = u.id "
            "ORDER BY i.name");

        while (query.next()) {
            ingredients.append(mapToIngredient(query.record()));
        }
        return ingredients;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении всех ингредиентов:" << e.what();
        throw;
    }
}

QList<Ingredient> IngredientRepository::getIngredientsByName(const QString& name) {
    QList<Ingredient> ingredients;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["name"] = "%" + name + "%";

        QSqlQuery query = dbContext->executeQuery(
            "SELECT i.*, u.name AS unit_name FROM ingredients i "
            "LEFT JOIN units u ON i.default_unit_id = u.id "
            "WHERE i.name ILIKE :name "
            "ORDER BY i.name", params);

        while (query.next()) {
            ingredients.append(mapToIngredient(query.record()));
        }
        return ingredients;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при поиске ингредиентов по имени:" << e.what();
        throw;
    }
}

QList<std::pair<int, double>> IngredientRepository::getIngredientSubstitutes(int ingredientId, double threshold, int limit) {
    QList<std::pair<int, double>> substitutes;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Проверка существования ингредиента
        auto ingredient = getIngredientById(ingredientId);
        if (!ingredient) {
            throw std::invalid_argument("Ингредиент не найден");
        }

        QVariantMap params;
        params["ingredient_id"] = ingredientId;
        params["threshold"] = threshold;
        params["limit"] = limit > 0 ? limit : 1000; // Если лимит не задан, используем большое значение

        // Запрос заменителей с учетом порога похожести
        QSqlQuery query = dbContext->executeQuery(
            "SELECT ingredient_id_2 AS id, similarity_score "
            "FROM ingredient_similarity "
            "WHERE ingredient_id_1 = :ingredient_id AND similarity_score >= :threshold "
            "ORDER BY similarity_score DESC "
            "LIMIT :limit", params);

        while (query.next()) {
            int substituteId = query.value("id").toInt();
            double score = query.value("similarity_score").toDouble();
            substitutes.append(std::make_pair(substituteId, score));
        }

        // Если ничего не найдено в первом направлении, проверяем обратное направление
        if (substitutes.isEmpty()) {
            QVariantMap reverseParams;
            reverseParams["ingredient_id"] = ingredientId;
            reverseParams["threshold"] = threshold;
            reverseParams["limit"] = params["limit"];

            QSqlQuery reverseQuery = dbContext->executeQuery(
                "SELECT ingredient_id_1 AS id, similarity_score "
                "FROM ingredient_similarity "
                "WHERE ingredient_id_2 = :ingredient_id AND similarity_score >= :threshold "
                "ORDER BY similarity_score DESC "
                "LIMIT :limit", reverseParams);

            while (reverseQuery.next()) {
                int substituteId = reverseQuery.value("id").toInt();
                double score = reverseQuery.value("similarity_score").toDouble();
                substitutes.append(std::make_pair(substituteId, score));
            }
        }

        return substitutes;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении заменителей ингредиента:" << e.what();
        throw;
    }
}

Ingredient IngredientRepository::mapToIngredient(const QSqlRecord& record) {
    int ingredientId = record.value("id").toInt();
    QString name = record.value("name").toString();
    QString defaultUnit = record.value("unit_name").toString();
    double calories = record.value("calories").toDouble();

    // В таблице нет поля default_expiration_days, но оно есть в классе Ingredient
    // Используем значение по умолчанию или вычисляем его на основе типа ингредиента
    int defaultExpirationDays = 7; // значение по умолчанию

    return Ingredient(ingredientId, name, defaultUnit, calories, defaultExpirationDays);
}
