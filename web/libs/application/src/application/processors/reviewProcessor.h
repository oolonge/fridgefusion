#ifndef REVIEWPROCESSOR_H
#define REVIEWPROCESSOR_H

#include "application/interfaces/processors/IReviewProcessor.h"
#include "infrastructure/repositories/IReviewRepository.h"

class ReviewProcessor : public IReviewProcessor {
public:
    explicit ReviewProcessor(IReviewRepository* reviewRepository);
    ~ReviewProcessor() override = default;

    QList<Review> getRecipeReviews(int recipeId) override;
    QList<Review> getUserReviews(int userId) override;
    bool addReview(int userId, int recipeId, int rating, const QString& comment) override;
    bool updateReview(int reviewId, int rating, const QString& comment) override;
    bool deleteReview(int reviewId) override;
    double getRecipeAverageRating(int recipeId) override;

private:
    IReviewRepository* reviewRepository;
};

#endif // REVIEWPROCESSOR_H
