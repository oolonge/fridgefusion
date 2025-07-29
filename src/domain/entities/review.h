#ifndef REVIEW_H
#define REVIEW_H

#include <QString>
#include <QDateTime>

class Review {
public:
    Review() = default;
    Review(int id, int userId, int recipeId, int rating, const QString& comment, const QDateTime& datePosted)
        : id(id), userId(userId), recipeId(recipeId), rating(rating), comment(comment), datePosted(datePosted) {}
    Review(int id, int userId, int recipeId, int rating, const QString& comment, const QDateTime& datePosted, const QString& username)
        : id(id), userId(userId), recipeId(recipeId), rating(rating), comment(comment), datePosted(datePosted), username(username) {}

    int getId() const { return id; }
    int getUserId() const { return userId; }
    int getRecipeId() const { return recipeId; }
    int getRating() const { return rating; }
    QString getComment() const { return comment; }
    QDateTime getDatePosted() const { return datePosted; }
    QString getUsername() const { return username; }

private:
    int id = 0;
    int userId = 0;
    int recipeId = 0;
    int rating = 0;
    QString comment;
    QDateTime datePosted;
    QString username;
};

#endif // REVIEW_H
