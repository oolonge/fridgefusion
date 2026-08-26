#ifndef USERPROFILEDTO_H
#define USERPROFILEDTO_H

#include <QString>
#include <QDateTime>
#include "domain/entities/user.h"

class UserProfileDTO {
public:
    UserProfileDTO() = default;

    // Создание DTO из сущности User
    explicit UserProfileDTO(const User& user)
        : id(user.getId()),
        username(user.getUsername()),
        email(user.getEmail()),
        registrationDate(user.getRegistrationDate()),
        role(user.getRole()) {}

    int id = 0;
    QString username;
    QString email;
    QDateTime registrationDate;
    UserRole role = UserRole::USER;
    int favoriteRecipesCount = 0;
    int reviewsCount = 0;
};

#endif // USERPROFILEDTO_H
