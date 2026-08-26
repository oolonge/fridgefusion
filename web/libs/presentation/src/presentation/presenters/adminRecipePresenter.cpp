#include "adminRecipePresenter.h"
#include <QDebug>

AdminRecipePresenter::AdminRecipePresenter(
    std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor,
    std::shared_ptr<IAuthProcessor> authProcessor,
    std::shared_ptr<IAdminProcessor> adminProcessor,
    AdminRecipeView* view)
    : recipeProcessor(std::move(recipeProcessor)),
    authProcessor(std::move(authProcessor)),
    adminProcessor(std::move(adminProcessor)),
    view(view) {
}

void AdminRecipePresenter::loadAllRecipes(int limit, int offset) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        QList<RecipePreviewDTO> recipePreviews = recipeProcessor->getAllRecipePreviews(limit, offset);
        view->displayRecipes(recipePreviews);
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при загрузке списка рецептов: %1").arg(e.what()));
    }
}

void AdminRecipePresenter::searchRecipes(const QString& searchTerm) {
    if (!checkAdminAccess()) {
        return;
    }

    if (searchTerm.isEmpty()) {
        loadAllRecipes();
        return;
    }

    try {
        // Здесь должен быть метод поиска рецептов по имени, который нужно будет реализовать
        // Пока просто загружаем все рецепты
        QList<RecipePreviewDTO> recipePreviews = recipeProcessor->getAllRecipePreviews();
        view->displayRecipes(recipePreviews);
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при поиске рецептов: %1").arg(e.what()));
    }
}

void AdminRecipePresenter::loadRecipeDetails(int recipeId) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        RecipeDetailDTO recipeDetail = recipeProcessor->getRecipeDetail(recipeId);
        view->displayRecipeDetails(recipeDetail);
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при загрузке деталей рецепта: %1").arg(e.what()));
    }
}

void AdminRecipePresenter::updateRecipe(int recipeId) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        // Получаем данные из формы представления
        RecipeFormData formData = view->getRecipeFormData();
        
        // Проверяем корректность данных
        if (formData.name.trimmed().isEmpty()) {
            view->showError("Название рецепта не может быть пустым");
            return;
        }
        
        if (formData.preparationTime <= 0 || formData.cookingTime <= 0) {
            view->showError("Время приготовления должно быть положительным");
            return;
        }
        
        // Обновляем рецепт через админ-процессор
        bool success = adminProcessor->updateRecipe(
            recipeId, 
            formData.name, 
            formData.description, 
            formData.preparationTime, 
            formData.cookingTime, 
            formData.instructions);
            
        // Отображаем результат операции
        view->showRecipeUpdatedStatus(
            success, 
            success ? QString("Рецепт успешно обновлен") : QString("Не удалось обновить рецепт")
        );
        
        // Важно: НЕ вызываем loadRecipeDetails, чтобы избежать бесконечного цикла
        // Данные в форме уже отражают то, что пользователь хотел изменить
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при обновлении рецепта: %1").arg(e.what()));
    }
}

void AdminRecipePresenter::deleteRecipe(int recipeId) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        // Удаляем рецепт через админ-процессор
        bool success = adminProcessor->deleteRecipe(recipeId);
        
        view->showRecipeDeletedStatus(
            success,
            success ? QString("Рецепт успешно удален") : QString("Не удалось удалить рецепт")
        );
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при удалении рецепта: %1").arg(e.what()));
    }
}

void AdminRecipePresenter::loadCategories() {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        QList<Category> categories = recipeProcessor->getAllCategories();
        view->displayCategories(categories);
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при загрузке категорий: %1").arg(e.what()));
    }
}

void AdminRecipePresenter::loadIngredients() {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        QList<Ingredient> ingredients = recipeProcessor->getAllIngredients();
        view->displayIngredients(ingredients);
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при загрузке ингредиентов: %1").arg(e.what()));
    }
}

void AdminRecipePresenter::addCategoryToRecipe(int recipeId, int categoryId) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        bool success = adminProcessor->addCategoryToRecipe(recipeId, categoryId);
        
        if (success) {
            // Обновляем детали рецепта для отображения новой категории
            // Здесь безопасно вызывать, т.к. это не связано с изменением ингредиентов
            loadRecipeDetails(recipeId);
        } else {
            view->showError("Не удалось добавить категорию к рецепту");
        }
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при добавлении категории к рецепту: %1").arg(e.what()));
    }
}

void AdminRecipePresenter::removeCategoryFromRecipe(int recipeId, int categoryId) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        bool success = adminProcessor->removeCategoryFromRecipe(recipeId, categoryId);
        
        if (success) {
            // Обновляем детали рецепта, чтобы отразить удаление категории
            // Здесь безопасно вызывать, т.к. это не связано с изменением ингредиентов
            loadRecipeDetails(recipeId);
        } else {
            view->showError("Не удалось удалить категорию из рецепта");
        }
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при удалении категории из рецепта: %1").arg(e.what()));
    }
}

void AdminRecipePresenter::addIngredientToRecipe(int recipeId, int ingredientId, double quantity) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        // Получаем выбранную единицу измерения из представления
        QString unit = view->getSelectedUnit();

        bool success = adminProcessor->addIngredientToRecipe(recipeId, ingredientId, quantity, unit);

        if (success) {
            loadRecipeDetails(recipeId);
        } else {
            view->showError("Не удалось добавить ингредиент к рецепту");
        }
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при добавлении ингредиента к рецепту: %1").arg(e.what()));
    }
}

void AdminRecipePresenter::removeIngredientFromRecipe(int recipeId, int ingredientId) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        bool success = adminProcessor->removeIngredientFromRecipe(recipeId, ingredientId);
        
        if (success) {
            // Обновляем детали рецепта после удаления ингредиента
            // Здесь нужно обновить, потому что элемент удаляется из таблицы
            loadRecipeDetails(recipeId);
        } else {
            view->showError("Не удалось удалить ингредиент из рецепта");
        }
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при удалении ингредиента из рецепта: %1").arg(e.what()));
    }
}

void AdminRecipePresenter::updateIngredientQuantity(int recipeId, int ingredientId, double quantity) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        bool success = adminProcessor->updateIngredientQuantity(recipeId, ingredientId, quantity);
        
        if (!success) {
            view->showError("Не удалось обновить количество ингредиента");
        }
        
        // Не вызываем loadRecipeDetails, чтобы избежать бесконечного цикла
        // Изменения уже отражены в интерфейсе
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при обновлении количества ингредиента: %1").arg(e.what()));
    }
}

bool AdminRecipePresenter::checkAdminAccess() {
    if (!authProcessor->isUserLoggedIn()) {
        view->showError("Необходимо войти в систему для доступа к администрированию");
        view->switchToMain();
        return false;
    }

    if (!authProcessor->isCurrentUserAdmin()) {
        view->showError("У вас нет прав администратора для выполнения этой операции");
        view->switchToMain();
        return false;
    }

    return true;
}

void AdminRecipePresenter::createRecipe(const QString& name) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        // Получаем ID текущего пользователя
        int currentUserId = authProcessor->getCurrentUserId();
        if (currentUserId <= 0) {
            view->showError("Ошибка: не удалось определить текущего пользователя");
            return;
        }

        // Для новых рецептов устанавливаем минимальные начальные значения
        int newRecipeId = adminProcessor->createRecipe(
            name,                           // Название рецепта
            "Описание рецепта",             // Описание по умолчанию
            10,                             // Время подготовки по умолчанию
            20,                             // Время приготовления по умолчанию
            "Инструкции по приготовлению",  // Инструкции по умолчанию
            currentUserId                   // ID текущего пользователя как автора
            );

        if (newRecipeId > 0) {
            view->showMessage("Рецепт успешно создан! Теперь вы можете отредактировать его детали.");

            // Перезагружаем список рецептов
            loadAllRecipes();

            // Сразу загружаем детали нового рецепта для редактирования
            loadRecipeDetails(newRecipeId);
        } else {
            view->showError("Не удалось создать рецепт. Пожалуйста, попробуйте еще раз.");
        }
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при создании рецепта: %1").arg(e.what()));
    }
}
