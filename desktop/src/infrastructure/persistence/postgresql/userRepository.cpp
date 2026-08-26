#include "userRepository.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <stdexcept>
#include "utils/logging/LoggerService.h"

UserRepository::UserRepository(std::shared_ptr<DbContext> dbContext, std::shared_ptr<ILogger> logger)
    : dbContext(std::move(dbContext)),
    logger(logger ? logger : LoggerService::getInstance().getLogger("default")) {
}

std::optional<User> UserRepository::getUserById(int userId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            logger->error(QString("Не удалось подключиться к базе данных при получении пользователя по ID: %1").arg(userId));
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["id"] = userId;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT * FROM users WHERE id = :id", params);

        if (query.next()) {
            return mapToUser(query.record());
        }
        return std::nullopt;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении пользователя по ID: %1: %2").arg(userId).arg(e.what()));
        throw;
    }
}

std::optional<User> UserRepository::getUserByEmail(const QString& email) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            logger->error(QString("Не удалось подключиться к базе данных при получении пользователя по email: %1").arg(email));
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["email"] = email;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT * FROM users WHERE email = :email", params);

        if (query.next()) {
            return mapToUser(query.record());
        }
        return std::nullopt;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении пользователя по email: %1: %2").arg(email).arg(e.what()));
        throw;
    }
}

QList<User> UserRepository::getUsersByUsername(const QString& username) {
    QList<User> users;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            logger->error("Не удалось подключиться к базе данных при поиске пользователей по имени");
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["username"] = "%" + username + "%";

        QSqlQuery query = dbContext->executeQuery(
            "SELECT * FROM users WHERE username ILIKE :username", params);

        while (query.next()) {
            users.append(mapToUser(query.record()));
        }
        return users;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при поиске пользователей по имени '%1': %2").arg(username).arg(e.what()));
        throw;
    }
}

QList<User> UserRepository::getUsersByRole(UserRole role) {
    QList<User> users;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            logger->error("Не удалось подключиться к базе данных при получении пользователей по роли");
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["role"] = userRoleToString(role);

        QSqlQuery query = dbContext->executeQuery(
            "SELECT * FROM users WHERE role = :role::user_role ORDER BY id", params);

        while (query.next()) {
            users.append(mapToUser(query.record()));
        }

        logger->debug(QString("Получено %1 пользователей с ролью %2").arg(users.size()).arg(userRoleToString(role)));
        return users;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении пользователей по роли %1: %2").arg(userRoleToString(role)).arg(e.what()));
        throw;
    }
}

QList<User> UserRepository::getAllUsers(int limit, int offset) {
    QList<User> users;
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            logger->error("Не удалось подключиться к базе данных при получении всех пользователей");
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["limit"] = limit;
        params["offset"] = offset;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT * FROM users ORDER BY id LIMIT :limit OFFSET :offset", params);

        while (query.next()) {
            users.append(mapToUser(query.record()));
        }

        logger->debug(QString("Получено %1 пользователей (limit: %2, offset: %3)").arg(users.size()).arg(limit).arg(offset));
        return users;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении всех пользователей: %1").arg(e.what()));
        throw;
    }
}

bool UserRepository::updateUserRole(int userId, UserRole role) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            logger->error("Не удалось подключиться к базе данных при обновлении роли пользователя");
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["id"] = userId;
        params["role"] = userRoleToString(role);

        QSqlQuery query = dbContext->executeQuery(
            "UPDATE users SET role = :role::user_role WHERE id = :id", params);

        bool success = query.numRowsAffected() > 0;

        if (success) {
            logger->info(QString("Обновлена роль пользователя %1 на %2").arg(userId).arg(userRoleToString(role)));
        } else {
            logger->warn(QString("Не удалось обновить роль пользователя %1 на %2").arg(userId).arg(userRoleToString(role)));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при обновлении роли пользователя %1: %2").arg(userId).arg(e.what()));
        throw;
    }
}

bool UserRepository::saveUser(User& user) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            logger->error("Не удалось подключиться к базе данных при сохранении пользователя");
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Валидация входных данных
        if (user.getUsername().isEmpty() || user.getEmail().isEmpty() || user.getPasswordHash().isEmpty()) {
            logger->error("Отсутствуют обязательные поля пользователя при сохранении");
            throw std::invalid_argument("Отсутствуют обязательные поля пользователя");
        }

        // Проверка на уникальность email
        if (getUserByEmail(user.getEmail())) {
            logger->warn(QString("Пользователь с email %1 уже существует").arg(user.getEmail()));
            throw std::invalid_argument("Пользователь с таким email уже существует");
        }

        QVariantMap userData;
        userData["username"] = user.getUsername();
        userData["email"] = user.getEmail();
        userData["password_hash"] = user.getPasswordHash();
        userData["registration_date"] = user.getRegistrationDate();
        userData["role"] = userRoleToString(user.getRole());

        return dbContext->transactional([&]() {
            QSqlQuery query = dbContext->executeQuery(
                "INSERT INTO users (username, email, password_hash, registration_date, role) "
                "VALUES (:username, :email, :password_hash, :registration_date, :role::user_role) RETURNING id",
                userData);

            if (query.next()) {
                user.setId(query.value(0).toInt());
                logger->info(QString("Сохранен новый пользователь %1 с ID %2 и ролью %3")
                                 .arg(user.getUsername())
                                 .arg(user.getId())
                                 .arg(userRoleToString(user.getRole())));
                return true;
            }

            logger->error("Не удалось получить ID нового пользователя");
            return false;
        });
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при сохранении пользователя: %1").arg(e.what()));
        throw;
    }
}

bool UserRepository::updateUser(const User& user) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            logger->error("Не удалось подключиться к базе данных при обновлении пользователя");
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Валидация входных данных
        if (user.getId() <= 0 || user.getUsername().isEmpty() || user.getEmail().isEmpty()) {
            logger->error("Отсутствуют обязательные поля пользователя при обновлении");
            throw std::invalid_argument("Отсутствуют обязательные поля пользователя");
        }

        // Проверка существования пользователя
        auto existingUser = getUserById(user.getId());
        if (!existingUser) {
            logger->warn(QString("Пользователь с ID %1 не найден при обновлении").arg(user.getId()));
            throw std::invalid_argument("Пользователь не найден");
        }

        // Проверка на уникальность email при его изменении
        if (existingUser->getEmail() != user.getEmail()) {
            auto userWithSameEmail = getUserByEmail(user.getEmail());
            if (userWithSameEmail && userWithSameEmail->getId() != user.getId()) {
                logger->warn(QString("Email %1 уже используется другим пользователем").arg(user.getEmail()));
                throw std::invalid_argument("Пользователь с таким email уже существует");
            }
        }

        QVariantMap userData;
        userData["id"] = user.getId();
        userData["username"] = user.getUsername();
        userData["email"] = user.getEmail();
        userData["password_hash"] = user.getPasswordHash();
        userData["role"] = userRoleToString(user.getRole());

        QString sql = "UPDATE users SET "
                      "username = :username, "
                      "email = :email, "
                      "password_hash = :password_hash, "
                      "role = :role::user_role "
                      "WHERE id = :id";

        QSqlQuery query = dbContext->executeQuery(sql, userData);

        bool success = query.numRowsAffected() > 0;

        if (success) {
            logger->info(QString("Обновлен пользователь с ID %1").arg(user.getId()));
        } else {
            logger->warn(QString("Не удалось обновить пользователя с ID %1").arg(user.getId()));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при обновлении пользователя: %1").arg(e.what()));
        throw;
    }
}

bool UserRepository::deleteUser(int userId) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            logger->error("Не удалось подключиться к базе данных при удалении пользователя");
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        // Проверка существования пользователя
        auto user = getUserById(userId);
        if (!user) {
            logger->warn(QString("Пользователь с ID %1 не найден при удалении").arg(userId));
            throw std::invalid_argument("Пользователь не найден");
        }

        return dbContext->transactional([&]() {
            // Удаляем связанные данные (отзывы и избранное)
            dbContext->remove("favorites", QString("user_id = %1").arg(userId));
            dbContext->remove("reviews", QString("user_id = %1").arg(userId));

            // Удаляем самого пользователя
            bool success = dbContext->remove("users", QString("id = %1").arg(userId));

            if (success) {
                logger->info(QString("Удален пользователь с ID %1").arg(userId));
            } else {
                logger->warn(QString("Не удалось удалить пользователя с ID %1").arg(userId));
            }

            return success;
        });
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при удалении пользователя: %1").arg(e.what()));
        throw;
    }
}

bool UserRepository::verifyCredentials(const QString& email, const QString& passwordHash) {
    try {
        if (!dbContext->isConnected() && !dbContext->connect()) {
            logger->error("Не удалось подключиться к базе данных при проверке учетных данных");
            throw std::runtime_error("Не удалось подключиться к базе данных");
        }

        QVariantMap params;
        params["email"] = email;
        params["password_hash"] = passwordHash;

        QSqlQuery query = dbContext->executeQuery(
            "SELECT COUNT(*) FROM users WHERE email = :email AND password_hash = :password_hash", params);

        if (query.next()) {
            bool verified = query.value(0).toInt() > 0;

            if (verified) {
                logger->debug(QString("Успешная верификация учетных данных для %1").arg(email));
            } else {
                logger->debug(QString("Неудачная верификация учетных данных для %1").arg(email));
            }

            return verified;
        }
        return false;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при проверке учетных данных: %1").arg(e.what()));
        throw;
    }
}

User UserRepository::mapToUser(const QSqlRecord& record) {
    int id = record.value("id").toInt();
    QString username = record.value("username").toString();
    QString email = record.value("email").toString();
    QString passwordHash = record.value("password_hash").toString();
    QDateTime registrationDate = record.value("registration_date").toDateTime();

    // Получаем роль из БД и преобразуем её в enum
    QString roleStr = record.value("role").toString();
    UserRole role = stringToUserRole(roleStr);

    return User(id, username, email, passwordHash, registrationDate, role);
}
