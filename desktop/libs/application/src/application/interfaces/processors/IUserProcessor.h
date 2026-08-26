#ifndef IUSERPROCESSOR_H
#define IUSERPROCESSOR_H

#include "domain/entities/user.h"
#include "domain/dtos/userProfileDTO.h"
#include <optional>
#include <QList>
#include <QString>

class IUserProcessor {
public:
    virtual ~IUserProcessor() = default;

    // Получение информации о пользователе
    virtual std::optional<User> getUserById(int userId) = 0;
    virtual std::optional<User> getUserByEmail(const QString& email) = 0;
    virtual QList<User> getUsersByUsername(const QString& username) = 0;

    // Получение пользователей по роли и всех пользователей
    virtual QList<User> getUsersByRole(UserRole role) = 0;
    virtual QList<User> getAllUsers(int limit = 100, int offset = 0) = 0;

    // Управление профилем пользователя
    virtual bool updateUserProfile(const User& user) = 0;
    virtual bool updateUserRole(int userId, UserRole role) = 0;
    virtual bool deleteUser(int userId) = 0;

    virtual std::optional<UserProfileDTO> getUserProfile(int userId) = 0;
};

#endif // IUSERPROCESSOR_H
