#ifndef IREVIEWREPOSITORY_H
#define IREVIEWREPOSITORY_H

#include <QList>
#include "domain/entities/review.h"

class IReviewRepository {
public:
    virtual ~IReviewRepository() = default;

    virtual QList<Review> getReviewsByRecipeId(int recipeId) = 0;
    virtual QList<Review> getReviewsByUserId(int userId) = 0;
    virtual bool addReview(const Review& review) = 0;
    virtual bool updateReview(const Review& review) = 0;
    virtual bool deleteReview(int reviewId) = 0;
    virtual double getAverageRatingForRecipe(int recipeId) = 0;
};

#endif // IREVIEWREPOSITORY_H
