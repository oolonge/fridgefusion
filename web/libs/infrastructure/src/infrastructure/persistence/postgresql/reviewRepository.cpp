#include "reviewRepository.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <stdexcept>

ReviewRepository::ReviewRepository(DbContext* dbContext)
    : dbContext(dbContext) {
}

bool ReviewRepository::deleteReview(int reviewId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Проверка существования отзыва
        QVariantMap checkParams;
        checkParams["id"] = reviewId;

        QSqlQuery checkQuery = dbContext->executeQuery("SELECT COUNT(*) FROM reviews WHERE id = :id", checkParams);
        if (checkQuery.next() && checkQuery.value(0).toInt() == 0) {
            throw std::invalid_argument("Отзыв с указанным ID не найден");
        }

        QString whereClause = QString("id = %1").arg(reviewId);
        return dbContext->remove("reviews", whereClause);
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при удалении отзыва:" << e.what();
        throw;
    }
}

double ReviewRepository::getAverageRatingForRecipe(int recipeId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["recipe_id"] = recipeId;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT AVG(rating) as avg_rating FROM reviews WHERE recipe_id = :recipe_id", params);

        if (query.next()) {
            return query.value("avg_rating").toDouble();
        }
        return 0.0;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении среднего рейтинга для рецепта:" << e.what();
        throw;
    }
}

Review ReviewRepository::mapToReview(const QSqlRecord& record) {
    int id = record.value("id").toInt();
    int userId = record.value("user_id").toInt();
    int recipeId = record.value("recipe_id").toInt();
    int rating = record.value("rating").toInt();
    QString comment = record.value("comment").toString();
    QDateTime datePosted = record.value("date_posted").toDateTime();
    QString username = record.value("username").toString();

    return Review(id, userId, recipeId, rating, comment, datePosted, username);
}

QList<Review> ReviewRepository::getReviewsByRecipeId(int recipeId) {
    QList<Review> reviews;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["recipe_id"] = recipeId;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT r.*, u.username FROM reviews r "
            "JOIN users u ON r.user_id = u.id "
            "WHERE r.recipe_id = :recipe_id ORDER BY r.date_posted DESC", params);

        while (query.next()) {
            reviews.append(mapToReview(query.record()));
        }
        return reviews;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении отзывов для рецепта:" << e.what();
        throw;
    }
}

QList<Review> ReviewRepository::getReviewsByUserId(int userId) {
    QList<Review> reviews;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["user_id"] = userId;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT * FROM reviews WHERE user_id = :user_id ORDER BY date_posted DESC", params);

        while (query.next()) {
            reviews.append(mapToReview(query.record()));
        }
        return reviews;
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при получении отзывов пользователя:" << e.what();
        throw;
    }
}

bool ReviewRepository::addReview(const Review& review) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Проверка рейтинга
        int rating = review.getRating();
        if (rating < 1 || rating > 5) {
            throw std::invalid_argument("Рейтинг должен быть от 1 до 5");
        }

        // Проверка на существование рецепта и пользователя можно добавить здесь

        // Проверка на наличие уже существующего отзыва от данного пользователя на данный рецепт
        QVariantMap checkParams;
        checkParams["user_id"] = review.getUserId();
        checkParams["recipe_id"] = review.getRecipeId();

        QSqlQuery checkQuery = dbContext->executeQuery(
            "SELECT id FROM reviews WHERE user_id = :user_id AND recipe_id = :recipe_id", checkParams);

        if (checkQuery.next()) {
            // Отзыв уже существует, обновляем его
            int existingReviewId = checkQuery.value("id").toInt();
            Review updatedReview(existingReviewId, review.getUserId(), review.getRecipeId(),
                                 rating, review.getComment(), review.getDatePosted());
            return updateReview(updatedReview);
        }

        // Добавляем новый отзыв
        QVariantMap reviewData;
        reviewData["user_id"] = review.getUserId();
        reviewData["recipe_id"] = review.getRecipeId();
        reviewData["rating"] = rating;
        reviewData["comment"] = review.getComment();
        reviewData["date_posted"] = review.getDatePosted();

        return dbContext->insert("reviews", reviewData);
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при добавлении отзыва:" << e.what();
        throw;
    }
}

bool ReviewRepository::updateReview(const Review& review) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Проверка существования отзыва
        QVariantMap checkParams;
        checkParams["id"] = review.getId();

        QSqlQuery checkQuery = dbContext->executeQuery("SELECT COUNT(*) FROM reviews WHERE id = :id", checkParams);
        if (checkQuery.next() && checkQuery.value(0).toInt() == 0) {
            throw std::invalid_argument("Отзыв с указанным ID не найден");
        }

        // Проверка рейтинга
        int rating = review.getRating();
        if (rating < 1 || rating > 5) {
            throw std::invalid_argument("Рейтинг должен быть от 1 до 5");
        }

        // Обновляем отзыв
        QVariantMap reviewData;
        reviewData["rating"] = rating;
        reviewData["comment"] = review.getComment();
        // Не обновляем user_id и recipe_id, так как они являются ключевыми полями

        QString whereClause = QString("id = %1").arg(review.getId());
        return dbContext->update("reviews", reviewData, whereClause);
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при обновлении отзыва:" << e.what();
        throw;
    }
}
