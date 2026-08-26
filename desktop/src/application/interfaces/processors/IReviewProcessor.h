#ifndef IREVIEWPROCESSOR_H
#define IREVIEWPROCESSOR_H

#include <QList>
#include <QString>
#include "domain/entities/review.h"

class IReviewProcessor {
public:
    virtual ~IReviewProcessor() = default;

    virtual QList<Review> getRecipeReviews(int recipeId) = 0;
    virtual QList<Review> getUserReviews(int userId) = 0;
    virtual bool addReview(int userId, int recipeId, int rating, const QString& comment) = 0;
    virtual bool updateReview(int reviewId, int rating, const QString& comment) = 0;
    virtual bool deleteReview(int reviewId) = 0;
    virtual double getRecipeAverageRating(int recipeId) = 0;
};

#endif // IREVIEWPROCESSOR_H
