#ifndef REVIEWREPOSITORY_H
#define REVIEWREPOSITORY_H

#include "infrastructure/repositories/IReviewRepository.h"
#include "infrastructure/persistence/database/dbContext.h"

class ReviewRepository : public IReviewRepository {
public:
    explicit ReviewRepository(DbContext* dbContext);
    ~ReviewRepository() override = default;

    QList<Review> getReviewsByRecipeId(int recipeId) override;
    QList<Review> getReviewsByUserId(int userId) override;
    bool addReview(const Review& review) override;
    bool updateReview(const Review& review) override;
    bool deleteReview(int reviewId) override;
    double getAverageRatingForRecipe(int recipeId) override;

private:
    DbContext* dbContext;

    // Вспомогательный метод для преобразования записи БД в объект
    Review mapToReview(const QSqlRecord& record);
};

#endif // REVIEWREPOSITORY_H
