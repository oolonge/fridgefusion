#include "userPresenter.h"
#include "domain/entities/user.h"
#include <QDebug>

UserPresenter::UserPresenter(
    std::shared_ptr<IUserProcessor> userProcessor,
    std::shared_ptr<IFavoriteProcessor> favoriteProcessor,
    std::shared_ptr<IRecipeExplorerProcessor> recipeExplorerProcessor, // Добавляем
    UserView* view)
    : userProcessor(std::move(userProcessor)),
    favoriteProcessor(std::move(favoriteProcessor)),
    recipeExplorerProcessor(std::move(recipeExplorerProcessor)), // Добавляем
    view(view) {
}

void UserPresenter::loadUserProfile(int userId) {
    auto profileOpt = userProcessor->getUserProfile(userId);

    if (profileOpt) {
        view->displayUserProfile(*profileOpt);
    }
}


void UserPresenter::loadFavoriteRecipes(int userId) {
    try {
        // Получаем список избранных рецептов пользователя
        QList<Favorite> favorites = favoriteProcessor->getUserFavorites(userId);

        // Отладочное сообщение
        qDebug() << "Получено избранных рецептов:" << favorites.size();

        // Преобразуем список Favorite в список RecipePreviewDTO
        QList<RecipePreviewDTO> favoriteRecipes;

        for (const Favorite& favorite : favorites) {
            int recipeId = favorite.getRecipeId();

            // Получаем превью рецепта
            RecipePreviewDTO recipePreview = recipeExplorerProcessor->getRecipePreview(recipeId);

            // Помечаем как избранное (важно для UI)
            recipePreview.isFavorite = true;

            favoriteRecipes.append(recipePreview);

            // Отладочное сообщение
            qDebug() << "Добавлен рецепт в избранное:" << recipePreview.name << "с ID:" << recipeId;
        }

        // Отображаем список избранных рецептов
        view->displayFavoriteRecipes(favoriteRecipes);

    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении избранных рецептов:" << e.what();

        // В случае ошибки отображаем пустой список
        QList<RecipePreviewDTO> emptyList;
        view->displayFavoriteRecipes(emptyList);
    }
}

void UserPresenter::updateUserProfile(int userId) {
    // Получаем данные из формы
    QString newUsername = view->getNewUsername();
    QString newEmail = view->getNewEmail();

    // Получаем текущего пользователя
    auto userOpt = userProcessor->getUserById(userId);

    if (!userOpt) {
        view->showProfileUpdateError("Пользователь не найден");
        return;
    }

    User updatedUser = *userOpt;
    updatedUser.setUsername(newUsername);
    updatedUser.setEmail(newEmail);

    // Обновляем профиль пользователя
    if (userProcessor->updateUserProfile(updatedUser)) {
        view->showProfileUpdateSuccess();
    } else {
        view->showProfileUpdateError("Не удалось обновить профиль");
    }
}

void UserPresenter::removeFromFavorites(int userId, int recipeId) {
    try {
        // Удаляем рецепт из избранного
        bool success = favoriteProcessor->removeFromFavorites(userId, recipeId);

        if (success) {
            // После успешного удаления обновляем список избранных рецептов
            loadFavoriteRecipes(userId);

            // Также обновляем профиль пользователя, чтобы обновился счетчик избранных
            loadUserProfile(userId);
        } else {
            qDebug() << "Не удалось удалить рецепт" << recipeId << "из избранного для пользователя" << userId;
        }
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при удалении рецепта из избранного:" << e.what();
    }
}
