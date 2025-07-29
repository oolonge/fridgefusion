#ifndef IUSERREPOSITORY_H
#define IUSERREPOSITORY_H

#include "domain/entities/user.h"
#include <QList>
#include <QString>
#include <optional>

class IUserRepository {
public:
    virtual ~IUserRepository() = default;

    virtual std::optional<User> getUserById(int userId) = 0;
    virtual std::optional<User> getUserByEmail(const QString& email) = 0;
    virtual QList<User> getUsersByUsername(const QString& username) = 0;

    virtual QList<User> getUsersByRole(UserRole role) = 0;
    virtual QList<User> getAllUsers(int limit = 100, int offset = 0) = 0;
    virtual bool updateUserRole(int userId, UserRole role) = 0;

    virtual bool saveUser(User& user) = 0;  // Принимает ссылку для обновления ID после сохранения
    virtual bool updateUser(const User& user) = 0;
    virtual bool deleteUser(int userId) = 0;

    virtual bool verifyCredentials(const QString& email, const QString& passwordHash) = 0;
};

#endif // IUSERREPOSITORY_H
