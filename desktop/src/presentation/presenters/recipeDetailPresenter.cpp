#include "recipeDetailPresenter.h"
#include <QMessageBox>

RecipeDetailPresenter::RecipeDetailPresenter(
    std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor,
    std::shared_ptr<IFavoriteProcessor> favoriteProcessor,
    std::shared_ptr<IReviewProcessor> reviewProcessor,
    std::shared_ptr<IAuthProcessor> authProcessor,
    RecipeDetailView* view)
    : recipeProcessor(std::move(recipeProcessor)),
    favoriteProcessor(std::move(favoriteProcessor)),
    reviewProcessor(std::move(reviewProcessor)),
    authProcessor(std::move(authProcessor)),
    view(view) {
}

void RecipeDetailPresenter::loadRecipeDetails(int recipeId) {
    RecipeDetailDTO recipe = recipeProcessor->getRecipeDetail(recipeId);

    // Проверяем, в избранном ли рецепт у текущего пользователя
    if (authProcessor->isUserLoggedIn()) {
        int userId = authProcessor->getCurrentUserId();
        recipe.isFavorite = favoriteProcessor->isRecipeFavorite(userId, recipeId);
    } else {
        recipe.isFavorite = false;
    }

    view->displayRecipeDetails(recipe);

    // Загружаем отзывы
    loadReviews(recipeId);
}

void RecipeDetailPresenter::loadReviews(int recipeId) {
    QList<Review> reviews = reviewProcessor->getRecipeReviews(recipeId);
    double averageRating = reviewProcessor->getRecipeAverageRating(recipeId);

    view->displayReviews(reviews, averageRating);
}

void RecipeDetailPresenter::toggleFavorite(int recipeId) {
    if (!authProcessor->isUserLoggedIn()) {
        QMessageBox::information(nullptr, "Избранное",
                                 "Для добавления рецепта в избранное необходимо авторизоваться");
        // НЕ меняем статус кнопки, сообщаем об этом представлению
        return;
    }

    int userId = authProcessor->getCurrentUserId();
    bool isFavorite = favoriteProcessor->isRecipeFavorite(userId, recipeId);
    bool success = false;

    if (isFavorite) {
        success = favoriteProcessor->removeFromFavorites(userId, recipeId);
        if (success) {
            view->updateFavoriteStatus(false);
        }
    } else {
        success = favoriteProcessor->addToFavorites(userId, recipeId);
        if (success) {
            view->updateFavoriteStatus(true);
        }
    }
}

void RecipeDetailPresenter::submitReview(int recipeId) {
    if (!authProcessor->isUserLoggedIn()) {
        QMessageBox::information(nullptr, "Отзыв",
                                 "Для добавления отзыва необходимо авторизоваться");
        return;
    }

    int userId = authProcessor->getCurrentUserId();
    int rating = view->getRating();
    QString reviewText = view->getReviewText();

    if (reviewText.isEmpty()) {
        QMessageBox::warning(nullptr, "Отзыв", "Пожалуйста, напишите текст отзыва");
        return;
    }

    if (reviewProcessor->addReview(userId, recipeId, rating, reviewText)) {
        QMessageBox::information(nullptr, "Отзыв", "Ваш отзыв успешно добавлен");
        loadReviews(recipeId); // Обновляем список отзывов
    } else {
        QMessageBox::warning(nullptr, "Отзыв", "Не удалось добавить отзыв");
    }
}
