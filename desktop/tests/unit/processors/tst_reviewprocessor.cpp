#include <QtTest>
#include "application/processors/reviewProcessor.h"

class MockReviewRepository : public IReviewRepository {
public:
    QList<Review> getReviewsByRecipeId(int recipeId) override {
        QList<Review> reviews;
        if (recipeId == 101) {
            reviews.append(Review(1, 1, 101, 5, "Great recipe!", QDateTime::currentDateTime()));
            reviews.append(Review(2, 2, 101, 4, "Very good!", QDateTime::currentDateTime()));
        }
        return reviews;
    }

    QList<Review> getReviewsByUserId(int userId) override {
        QList<Review> reviews;
        if (userId == 1) {
            reviews.append(Review(1, 1, 101, 5, "Great recipe!", QDateTime::currentDateTime()));
            reviews.append(Review(3, 1, 102, 3, "Average recipe", QDateTime::currentDateTime()));
        }
        return reviews;
    }

    bool addReview(const Review& review) override {
        return true;
    }

    bool updateReview(const Review& review) override {
        return review.getId() == 1;
    }

    bool deleteReview(int reviewId) override {
        return reviewId == 1;
    }

    double getAverageRatingForRecipe(int recipeId) override {
        if (recipeId == 101) {
            return 4.5; // (5 + 4) / 2
        }
        return 0.0;
    }
};

class ReviewProcessorTest : public QObject {
    Q_OBJECT

private slots:
    void testGetRecipeReviews();
    void testGetUserReviews();
    void testAddReview();
    void testUpdateReview();
    void testDeleteReview();
    void testGetRecipeAverageRating();
};

void ReviewProcessorTest::testGetRecipeReviews() {
    MockReviewRepository repository;
    ReviewProcessor processor(&repository);

    QList<Review> reviews = processor.getRecipeReviews(101);
    QCOMPARE(reviews.size(), 2);
    QCOMPARE(reviews[0].getRating(), 5);
    QCOMPARE(reviews[1].getRating(), 4);

    QList<Review> emptyReviews = processor.getRecipeReviews(999);
    QCOMPARE(emptyReviews.size(), 0);
}

void ReviewProcessorTest::testGetUserReviews() {
    MockReviewRepository repository;
    ReviewProcessor processor(&repository);

    QList<Review> reviews = processor.getUserReviews(1);
    QCOMPARE(reviews.size(), 2);
    QCOMPARE(reviews[0].getRecipeId(), 101);
    QCOMPARE(reviews[1].getRecipeId(), 102);

    QList<Review> emptyReviews = processor.getUserReviews(999);
    QCOMPARE(emptyReviews.size(), 0);
}

void ReviewProcessorTest::testAddReview() {
    MockReviewRepository repository;
    ReviewProcessor processor(&repository);

    QVERIFY(processor.addReview(1, 103, 5, "Excellent recipe!"));
    QVERIFY(!processor.addReview(1, 103, 6, "Invalid rating!")); // Рейтинг должен быть от 1 до 5
}

void ReviewProcessorTest::testUpdateReview() {
    MockReviewRepository repository;
    ReviewProcessor processor(&repository);

    QVERIFY(processor.updateReview(1, 4, "Updated comment"));
    QVERIFY(!processor.updateReview(999, 4, "Non-existent review"));
    QVERIFY(!processor.updateReview(1, 6, "Invalid rating")); // Рейтинг должен быть от 1 до 5
}

void ReviewProcessorTest::testDeleteReview() {
    MockReviewRepository repository;
    ReviewProcessor processor(&repository);

    QVERIFY(processor.deleteReview(1));
    QVERIFY(!processor.deleteReview(999));
}

void ReviewProcessorTest::testGetRecipeAverageRating() {
    MockReviewRepository repository;
    ReviewProcessor processor(&repository);

    QCOMPARE(processor.getRecipeAverageRating(101), 4.5);
    QCOMPARE(processor.getRecipeAverageRating(999), 0.0);
}

// QTEST_APPLESS_MAIN(ReviewProcessorTest)
#include "tst_reviewprocessor.moc"
