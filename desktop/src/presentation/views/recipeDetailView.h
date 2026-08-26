#ifndef RECIPEDETAILVIEW_H
#define RECIPEDETAILVIEW_H

#include <QList>
#include <QString>
#include "domain/dtos/recipeDetailDTO.h"
#include "domain/entities/review.h"

class RecipeDetailView {
public:
    virtual ~RecipeDetailView() = default;

    // Методы для обновления представления
    virtual void displayRecipeDetails(const RecipeDetailDTO& recipeDetail) = 0;
    virtual void displayReviews(const QList<Review>& reviews, double averageRating) = 0;
    virtual void updateFavoriteStatus(bool isFavorite) = 0;
    virtual void setReturnToProfile(bool value) = 0;

    // Методы для получения данных из формы
    virtual int getRating() = 0;
    virtual QString getReviewText() = 0;

    // Навигация
    virtual void goBack() = 0;
};

#endif // RECIPEDETAILVIEW_H
