#include "recipeRepository.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QStringList>
#include <stdexcept>
#include <QJsonArray>

RecipeRepository::RecipeRepository(std::shared_ptr<DbContext> dbContext)
    : dbContext(std::move(dbContext)) {
}

std::optional<Recipe> RecipeRepository::getRecipeById(int recipeId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["id"] = recipeId;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT * FROM recipes WHERE id = :id", params);

        if (query.next()) {
            return mapToRecipe(query.record());
        }
        return std::nullopt;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении рецепта по ID:" << e.what();
        throw;
    }
}

QList<Category> RecipeRepository::getAllCategories() {
    QList<Category> categories;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QSqlQuery query = dbContext->executeQuery("SELECT * FROM categories ORDER BY name");

        while (query.next()) {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();
            QString description = query.value("description").toString();

            categories.append(Category(id, name, description));
        }
        return categories;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении всех категорий:" << e.what();
        throw;
    }
}

QList<Recipe> RecipeRepository::getAllRecipes(int limit, int offset) {
    QList<Recipe> recipes;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["limit"] = limit;
        params["offset"] = offset;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT * FROM recipes ORDER BY name LIMIT :limit OFFSET :offset", params);

        while (query.next()) {
            recipes.append(mapToRecipe(query.record()));
        }
        return recipes;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении всех рецептов:" << e.what();
        throw;
    }
}

QList<Recipe> RecipeRepository::getRecipesByIngredients(const QList<int>& ingredientIds) {
    QList<Recipe> recipes;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        if (ingredientIds.isEmpty()) {
            return recipes;
        }

        // Формируем список ID ингредиентов для SQL-запроса
        QStringList ingredientIdStrings;
        for (int id : ingredientIds) {
            ingredientIdStrings.append(QString::number(id));
        }
        QString ingredientIdsList = ingredientIdStrings.join(',');

        // Запрос для получения рецептов, содержащих ВСЕ указанные ингредиенты
        // Используем GROUP BY и HAVING для проверки, что все ингредиенты присутствуют
        QString sql = QString(
                          "SELECT r.* FROM recipes r "
                          "JOIN recipe_ingredients ri ON r.id = ri.recipe_id "
                          "WHERE ri.ingredient_id IN (%1) "
                          "GROUP BY r.id "
                          "HAVING COUNT(DISTINCT ri.ingredient_id) = %2 "
                          "ORDER BY r.name"
                          ).arg(ingredientIdsList).arg(ingredientIds.size());

        QSqlQuery query = dbContext->executeQuery(sql);

        while (query.next()) {
            recipes.append(mapToRecipe(query.record()));
        }
        return recipes;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при поиске рецептов по ингредиентам:" << e.what();
        throw;
    }
}

QList<Recipe> RecipeRepository::getRecipesByCategory(int categoryId) {
    QList<Recipe> recipes;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["category_id"] = categoryId;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT r.* FROM recipes r "
            "JOIN recipe_categories rc ON r.id = rc.recipe_id "
            "WHERE rc.category_id = :category_id "
            "ORDER BY r.name", params);

        while (query.next()) {
            recipes.append(mapToRecipe(query.record()));
        }
        return recipes;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при поиске рецептов по категории:" << e.what();
        throw;
    }
}

QList<RecipeIngredient> RecipeRepository::getRecipeIngredients(int recipeId) {
    QList<RecipeIngredient> ingredients;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["recipe_id"] = recipeId;

        // Получаем ингредиенты с информацией о единицах измерения
        QSqlQuery query = dbContext->executeQuery(
            "SELECT ri.*, i.name as ingredient_name, u.name as unit_name "
            "FROM recipe_ingredients ri "
            "JOIN ingredients i ON ri.ingredient_id = i.id "
            "JOIN units u ON ri.unit_id = u.id "
            "WHERE ri.recipe_id = :recipe_id", params);

        while (query.next()) {
            ingredients.append(mapToRecipeIngredient(query.record()));
        }
        return ingredients;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении ингредиентов рецепта:" << e.what();
        throw;
    }
}

QList<std::pair<int, double>> RecipeRepository::getIngredientSubstitutes(int ingredientId, int limit) {
    QList<std::pair<int, double>> substitutes;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["ingredient_id"] = ingredientId;
        params["limit"] = limit > 0 ? limit : 3;

        // Запрос на получение заменителей ингредиента
        QSqlQuery query = dbContext->executeQuery(
            "SELECT ingredient_id_2 as substitute_id, similarity_score "
            "FROM ingredient_similarity "
            "WHERE ingredient_id_1 = :ingredient_id "
            "ORDER BY similarity_score DESC "
            "LIMIT :limit", params);

        while (query.next()) {
            int substituteId = query.value("substitute_id").toInt();
            double score = query.value("similarity_score").toDouble();
            substitutes.append(std::make_pair(substituteId, score));
        }

        // Если ничего не найдено в первом направлении, проверим обратное
        if (substitutes.isEmpty()) {
            QSqlQuery reverseQuery = dbContext->executeQuery(
                "SELECT ingredient_id_1 as substitute_id, similarity_score "
                "FROM ingredient_similarity "
                "WHERE ingredient_id_2 = :ingredient_id "
                "ORDER BY similarity_score DESC "
                "LIMIT :limit", params);

            while (reverseQuery.next()) {
                int substituteId = reverseQuery.value("substitute_id").toInt();
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

RecipePreviewDTO RecipeRepository::getRecipePreview(int recipeId) {
    RecipePreviewDTO preview;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Проверяем существование рецепта
        auto recipe = getRecipeById(recipeId);
        if (!recipe) {
            return preview;
        }

        // Базовая информация о рецепте
        preview.id = recipeId;
        preview.name = recipe->getName();
        preview.shortDescription = recipe->getDescription();
        preview.totalTime = recipe->getTotalTime();

        // Получаем основные ингредиенты (лимитируем до 5)
        QVariantMap params;
        params["recipe_id"] = recipeId;
        params["limit"] = 5;

        QSqlQuery ingredientsQuery = dbContext->executeQuery(
            "SELECT i.name "
            "FROM recipe_ingredients ri "
            "JOIN ingredients i ON ri.ingredient_id = i.id "
            "WHERE ri.recipe_id = :recipe_id "
            "ORDER BY ri.is_optional, ri.id "
            "LIMIT :limit", params);

        while (ingredientsQuery.next()) {
            preview.mainIngredients.append(ingredientsQuery.value("name").toString());
        }

        // Получаем средний рейтинг
        QSqlQuery ratingQuery = dbContext->executeQuery(
            "SELECT AVG(rating) as avg_rating "
            "FROM reviews "
            "WHERE recipe_id = :recipe_id", params);

        if (ratingQuery.next()) {
            preview.averageRating = ratingQuery.value("avg_rating").toDouble();
        }

        // Получаем имя автора рецепта
        QSqlQuery authorQuery = dbContext->executeQuery(
            "SELECT u.username "
            "FROM recipes r "
            "JOIN users u ON r.author_id = u.id "
            "WHERE r.id = :recipe_id", params);

        if (authorQuery.next()) {
            preview.authorName = authorQuery.value("username").toString();
        } else {
            preview.authorName = "Неизвестный автор";
        }

        // Признак "в избранном" по умолчанию false, так как требует ID пользователя
        preview.isFavorite = false;

        return preview;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении предпросмотра рецепта:" << e.what();
        throw;
    }
}

RecipeDetailDTO RecipeRepository::getRecipeDetail(int recipeId) {
    RecipeDetailDTO detail;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Проверяем существование рецепта
        auto recipe = getRecipeById(recipeId);
        if (!recipe) {
            return detail;
        }

        // Базовая информация о рецепте
        detail.id = recipeId;
        detail.name = recipe->getName();
        detail.fullDescription = recipe->getDescription();
        detail.preparationTime = recipe->getPreparationTime();
        detail.cookingTime = recipe->getCookingTime();

        // Получаем категории рецепта
        QVariantMap params;
        params["recipe_id"] = recipeId;

        QSqlQuery categoriesQuery = dbContext->executeQuery(
            "SELECT c.name "
            "FROM recipe_categories rc "
            "JOIN categories c ON rc.category_id = c.id "
            "WHERE rc.recipe_id = :recipe_id "
            "ORDER BY c.name", params);

        while (categoriesQuery.next()) {
            detail.categories.append(categoriesQuery.value("name").toString());
        }

        // Получаем информацию о каждом ингредиенте
        QSqlQuery ingredientsQuery = dbContext->executeQuery(
            "SELECT ri.*, i.name as ingredient_name, u.name as unit_name "
            "FROM recipe_ingredients ri "
            "JOIN ingredients i ON ri.ingredient_id = i.id "
            "JOIN units u ON ri.unit_id = u.id "
            "WHERE ri.recipe_id = :recipe_id", params);

        while (ingredientsQuery.next()) {
            RecipeDetailDTO::IngredientInfo info;
            info.id = ingredientsQuery.value("ingredient_id").toInt();
            info.name = ingredientsQuery.value("ingredient_name").toString(); // Правильное получение названия
            info.quantity = ingredientsQuery.value("quantity").toDouble();
            info.unit = ingredientsQuery.value("unit_name").toString();
            info.isOptional = ingredientsQuery.value("is_optional").toBool();

            // Получаем возможные заменители для ингредиента
            auto substitutes = getIngredientSubstitutes(info.id, 3);
            for (const auto& sub : substitutes) {
                // Получаем название заменителя
                auto substituteOpt = dbContext->executeQuery("SELECT name FROM ingredients WHERE id = " + QString::number(sub.first));
                if (substituteOpt.next()) {
                    info.possibleSubstitutes.append(std::make_pair(sub.first, substituteOpt.value("name").toString()));
                }
            }

            detail.ingredients.append(info);
        }

        // Получаем шаги приготовления
        detail.steps = recipe->getSteps();

        // Получаем средний рейтинг и количество отзывов
        QSqlQuery ratingQuery = dbContext->executeQuery(
            "SELECT AVG(rating) as avg_rating, COUNT(*) as review_count "
            "FROM reviews "
            "WHERE recipe_id = :recipe_id", params);

        if (ratingQuery.next()) {
            detail.averageRating = ratingQuery.value("avg_rating").toDouble();
            detail.reviewCount = ratingQuery.value("review_count").toInt();
        }

        // Признак "в избранном" по умолчанию false, так как требует ID пользователя
        detail.isFavorite = false;

        return detail;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении подробной информации о рецепте:" << e.what();
        throw;
    }
}

Recipe RecipeRepository::mapToRecipe(const QSqlRecord& record) {
    int recipeId = record.value("id").toInt();
    QString name = record.value("name").toString();
    QString description = record.value("description").toString();
    int preparationTime = record.value("preparation_time").toInt();
    int cookingTime = record.value("cooking_time").toInt();

    // Получаем категории рецепта
    QList<int> categoryIds = getRecipeCategoryIds(recipeId);

    // Получаем шаги приготовления
    QString instructionsStr = record.value("instructions").toString();
    QStringList steps = instructionsStr.split('\n', Qt::SkipEmptyParts);

    // Ингредиенты загружаем отдельно при необходимости
    QList<RecipeIngredient> ingredients;

    return Recipe(recipeId, name, description, preparationTime, cookingTime, categoryIds, steps, ingredients);
}

RecipeIngredient RecipeRepository::mapToRecipeIngredient(const QSqlRecord& record) {
    int recipeId = record.value("recipe_id").toInt();
    int ingredientId = record.value("ingredient_id").toInt();
    double quantity = record.value("quantity").toDouble();
    QString unit = record.value("unit_name").toString();
    bool isOptional = record.value("is_optional").toBool();

    return RecipeIngredient(recipeId, ingredientId, quantity, unit, isOptional);
}

QList<int> RecipeRepository::getRecipeCategoryIds(int recipeId) {
    QList<int> categoryIds;
    try {
        QVariantMap params;
        params["recipe_id"] = recipeId;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT category_id FROM recipe_categories WHERE recipe_id = :recipe_id", params);

        while (query.next()) {
            categoryIds.append(query.value("category_id").toInt());
        }
        return categoryIds;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении категорий рецепта:" << e.what();
        return categoryIds;
    }
}

int RecipeRepository::getUnitIdByName(const QString& unitName) {
    try {
        QVariantMap params;
        params["name"] = unitName;
        params["abbr"] = unitName;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT id FROM units WHERE name = :name OR abbreviation = :abbr", params);

        if (query.next()) {
            return query.value("id").toInt();
        }

        // Отладочное сообщение
        qDebug() << "Единица измерения не найдена:" << unitName << ". Используем граммы по умолчанию.";

        // Для более детальной отладки, выведем все доступные единицы измерения
        QSqlQuery allUnitsQuery = dbContext->executeQuery("SELECT id, name, abbreviation FROM units");
        qDebug() << "Доступные единицы измерения:";
        while (allUnitsQuery.next()) {
            qDebug() << "ID:" << allUnitsQuery.value("id").toInt()
                     << "Название:" << allUnitsQuery.value("name").toString()
                     << "Аббревиатура:" << allUnitsQuery.value("abbreviation").toString();
        }

        return 1; // Граммы по умолчанию
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении ID единицы измерения:" << e.what();
        return 1; // Граммы по умолчанию
    }
}

bool RecipeRepository::updateRecipe(int recipeId, const QString& name, const QString& description, 
                                 int preparationTime, int cookingTime, const QString& instructions) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["id"] = recipeId;
        params["name"] = name;
        params["description"] = description;
        params["preparation_time"] = preparationTime;
        params["cooking_time"] = cookingTime;
        params["instructions"] = instructions;

        QSqlQuery query = dbContext->executeQuery(
            "UPDATE recipes SET name = :name, description = :description, "
            "preparation_time = :preparation_time, cooking_time = :cooking_time, "
            "instructions = :instructions "
            "WHERE id = :id", params);

        return query.numRowsAffected() > 0;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при обновлении рецепта:" << e.what();
        return false;
    }
}

bool RecipeRepository::deleteRecipe(int recipeId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Начинаем транзакцию
        if (!dbContext->beginTransaction()) {
            throw std::runtime_error("Не удалось начать транзакцию");
        }

        try {
            QVariantMap params;
            params["recipe_id"] = recipeId;

            // Сначала удаляем связи с ингредиентами
            dbContext->executeQuery(
                "DELETE FROM recipe_ingredients WHERE recipe_id = :recipe_id", params);

            // Удаляем связи с категориями
            dbContext->executeQuery(
                "DELETE FROM recipe_categories WHERE recipe_id = :recipe_id", params);

            // Удаляем отзывы к рецепту
            dbContext->executeQuery(
                "DELETE FROM reviews WHERE recipe_id = :recipe_id", params);

            // Удаляем избранное
            dbContext->executeQuery(
                "DELETE FROM favorites WHERE recipe_id = :recipe_id", params);

            // Наконец удаляем сам рецепт
            params.clear();
            params["id"] = recipeId;
            QSqlQuery deleteQuery = dbContext->executeQuery(
                "DELETE FROM recipes WHERE id = :id", params);

            bool success = deleteQuery.numRowsAffected() > 0;
            
            // Завершаем транзакцию
            if (success) {
                dbContext->commitTransaction();
            } else {
                dbContext->rollbackTransaction();
            }
            
            return success;
        } catch (const std::exception& e) {
            dbContext->rollbackTransaction();
            throw;
        }
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при удалении рецепта:" << e.what();
        return false;
    }
}

bool RecipeRepository::addCategoryToRecipe(int recipeId, int categoryId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Проверяем, существует ли уже такая связь
        QVariantMap checkParams;
        checkParams["recipe_id"] = recipeId;
        checkParams["category_id"] = categoryId;

        QSqlQuery checkQuery = dbContext->executeQuery(
            "SELECT 1 FROM recipe_categories WHERE recipe_id = :recipe_id AND category_id = :category_id", 
            checkParams);

        if (checkQuery.next()) {
            // Связь уже существует
            return true;
        }

        // Добавляем связь
        QSqlQuery insertQuery = dbContext->executeQuery(
            "INSERT INTO recipe_categories (recipe_id, category_id) VALUES (:recipe_id, :category_id)", 
            checkParams);

        return insertQuery.numRowsAffected() > 0;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при добавлении категории к рецепту:" << e.what();
        return false;
    }
}

bool RecipeRepository::removeCategoryFromRecipe(int recipeId, int categoryId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["recipe_id"] = recipeId;
        params["category_id"] = categoryId;

        QSqlQuery query = dbContext->executeQuery(
            "DELETE FROM recipe_categories WHERE recipe_id = :recipe_id AND category_id = :category_id", 
            params);

        return query.numRowsAffected() > 0;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при удалении категории из рецепта:" << e.what();
        return false;
    }
}

bool RecipeRepository::addIngredientToRecipe(int recipeId, int ingredientId, double quantity, 
                                           const QString& unit, bool isOptional) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Получаем ID единицы измерения
        int unitId = getUnitIdByName(unit);

        // Проверяем, существует ли уже такой ингредиент в рецепте
        QVariantMap checkParams;
        checkParams["recipe_id"] = recipeId;
        checkParams["ingredient_id"] = ingredientId;

        QSqlQuery checkQuery = dbContext->executeQuery(
            "SELECT 1 FROM recipe_ingredients WHERE recipe_id = :recipe_id AND ingredient_id = :ingredient_id", 
            checkParams);

        if (checkQuery.next()) {
            // Ингредиент уже существует в рецепте, обновляем количество
            return updateIngredientQuantity(recipeId, ingredientId, quantity);
        }

        // Добавляем ингредиент
        QVariantMap insertParams;
        insertParams["recipe_id"] = recipeId;
        insertParams["ingredient_id"] = ingredientId;
        insertParams["quantity"] = quantity;
        insertParams["unit_id"] = unitId;
        insertParams["is_optional"] = isOptional;

        QSqlQuery insertQuery = dbContext->executeQuery(
            "INSERT INTO recipe_ingredients (recipe_id, ingredient_id, quantity, unit_id, is_optional) "
            "VALUES (:recipe_id, :ingredient_id, :quantity, :unit_id, :is_optional)", 
            insertParams);

        return insertQuery.numRowsAffected() > 0;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при добавлении ингредиента в рецепт:" << e.what();
        return false;
    }
}

bool RecipeRepository::removeIngredientFromRecipe(int recipeId, int ingredientId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["recipe_id"] = recipeId;
        params["ingredient_id"] = ingredientId;

        QSqlQuery query = dbContext->executeQuery(
            "DELETE FROM recipe_ingredients WHERE recipe_id = :recipe_id AND ingredient_id = :ingredient_id", 
            params);

        return query.numRowsAffected() > 0;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при удалении ингредиента из рецепта:" << e.what();
        return false;
    }
}

bool RecipeRepository::updateIngredientQuantity(int recipeId, int ingredientId, double quantity) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["recipe_id"] = recipeId;
        params["ingredient_id"] = ingredientId;
        params["quantity"] = quantity;

        QSqlQuery query = dbContext->executeQuery(
            "UPDATE recipe_ingredients SET quantity = :quantity "
            "WHERE recipe_id = :recipe_id AND ingredient_id = :ingredient_id", 
            params);

        return query.numRowsAffected() > 0;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при обновлении количества ингредиента:" << e.what();
        return false;
    }
}

int RecipeRepository::createRecipe(const QString& name, const QString& description,
                                   int preparationTime, int cookingTime,
                                   const QString& instructions, int authorId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Создаем новый рецепт с базовыми данными
        QVariantMap recipeData;
        recipeData["name"] = name;
        recipeData["description"] = description;
        recipeData["preparation_time"] = preparationTime;
        recipeData["cooking_time"] = cookingTime;
        recipeData["instructions"] = instructions;
        recipeData["author_id"] = authorId;

        bool success = dbContext->insert("recipes", recipeData);

        if (success) {
            // Получаем ID нового рецепта
            QSqlQuery query = dbContext->executeQuery("SELECT LASTVAL()");
            if (query.next()) {
                int newRecipeId = query.value(0).toInt();
                qDebug() << "Создан новый рецепт с ID:" << newRecipeId;
                return newRecipeId;
            }
        }

        return -1;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при создании рецепта:" << e.what();
        return -1;
    }
}
