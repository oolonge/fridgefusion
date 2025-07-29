#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "infrastructure/repositories/IUserRepository.h"
#include "infrastructure/persistence/database/dbContext.h"
#include "utils/logging/ILogger.h"
#include <memory>

class UserRepository : public IUserRepository {
public:
    explicit UserRepository(
        std::shared_ptr<DbContext> dbContext,
        std::shared_ptr<ILogger> logger = nullptr
        );
    ~UserRepository() override = default;

    // CRUD операции
    std::optional<User> getUserById(int userId) override;
    std::optional<User> getUserByEmail(const QString& email) override;
    QList<User> getUsersByUsername(const QString& username) override;

    // Операции с ролями
    QList<User> getUsersByRole(UserRole role) override;
    QList<User> getAllUsers(int limit = 100, int offset = 0) override;
    bool updateUserRole(int userId, UserRole role) override;

    bool saveUser(User& user) override;
    bool updateUser(const User& user) override;
    bool deleteUser(int userId) override;

    // Аутентификация
    bool verifyCredentials(const QString& email, const QString& passwordHash) override;

private:
    std::shared_ptr<DbContext> dbContext;
    std::shared_ptr<ILogger> logger;

    // Преобразование записи БД в объект
    User mapToUser(const QSqlRecord& record);
};

#endif // USERREPOSITORY_H
