#include "reviewProcessor.h"
#include <QDateTime>

ReviewProcessor::ReviewProcessor(IReviewRepository* reviewRepository)
    : reviewRepository(reviewRepository) {
}

QList<Review> ReviewProcessor::getRecipeReviews(int recipeId) {
    return reviewRepository->getReviewsByRecipeId(recipeId);
}

QList<Review> ReviewProcessor::getUserReviews(int userId) {
    return reviewRepository->getReviewsByUserId(userId);
}

bool ReviewProcessor::addReview(int userId, int recipeId, int rating, const QString& comment) {
    if (rating < 1 || rating > 5) {
        return false;
    }
    
    Review review(0, userId, recipeId, rating, comment, QDateTime::currentDateTime());
    return reviewRepository->addReview(review);
}

bool ReviewProcessor::updateReview(int reviewId, int rating, const QString& comment) {
    if (rating < 1 || rating > 5) {
        return false;
    }
    
    // Это упрощенная версия, в реальном коде нужно получить существующий обзор и изменить его
    Review review(reviewId, 0, 0, rating, comment, QDateTime::currentDateTime());
    return reviewRepository->updateReview(review);
}

bool ReviewProcessor::deleteReview(int reviewId) {
    return reviewRepository->deleteReview(reviewId);
}

double ReviewProcessor::getRecipeAverageRating(int recipeId) {
    return reviewRepository->getAverageRatingForRecipe(recipeId);
}
