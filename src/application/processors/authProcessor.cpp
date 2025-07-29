#include "authProcessor.h"
#include <QDebug>
#include <QDateTime>
#include "utils/logging/LoggerService.h"

AuthProcessor::AuthProcessor(
    std::shared_ptr<IUserRepository> userRepository,
    std::shared_ptr<IPasswordHasher> passwordHasher,
    std::shared_ptr<ILogger> logger)
    : userRepository(std::move(userRepository)),
    passwordHasher(std::move(passwordHasher)),
    logger(logger ? logger : LoggerService::getInstance().getLogger("default")),
    loggedIn(false) {
}

AuthResult AuthProcessor::login(const QString& email, const QString& password) {
    // Check if email exists
    auto user = userRepository->getUserByEmail(email);
    if (!user) {
        logger->warn(QString("Попытка входа с несуществующим email: %1").arg(email));
        return AuthResult(AuthErrorType::UserNotFound, "Пользователь с таким email не существует");
    }

    // Verify password
    if (!passwordHasher->verifyPassword(password, user->getPasswordHash())) {
        logger->warn(QString("Неверный пароль при входе для пользователя: %1").arg(email));
        return AuthResult(AuthErrorType::InvalidPassword, "Неверный пароль");
    }

    // Login successful
    currentUser = user;
    loggedIn = true;

    logger->info(QString("Успешный вход пользователя: %1 (ID: %2) с ролью %3")
        .arg(user->getUsername())
        .arg(user->getId())
        .arg(userRoleToString(user->getRole())));

    return AuthResult(*user);
}

AuthResult AuthProcessor::registerUser(const QString& username, const QString& email, const QString& password, UserRole role) {
    // Validate input
    if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
        logger->warn("Попытка регистрации с пустыми обязательными полями");
        return AuthResult(AuthErrorType::InvalidInput, "Имя пользователя, email и пароль не могут быть пустыми");
    }

    // Check if email already exists
    if (userRepository->getUserByEmail(email)) {
        logger->warn(QString("Попытка регистрации с существующим email: %1").arg(email));
        return AuthResult(AuthErrorType::EmailAlreadyExists, "Email уже зарегистрирован");
    }

    // Create new user
    QString hashedPassword = passwordHasher->hashPassword(password);
    User newUser(0, username, email, hashedPassword, QDateTime::currentDateTime(), role);

    try {
        // Save user to database
        if (!userRepository->saveUser(newUser)) {
            logger->error(QString("Не удалось зарегистрировать пользователя: %1").arg(email));
            return AuthResult(AuthErrorType::RegistrationFailed, "Не удалось зарегистрировать пользователя");
        }

        // Log in the new user
        currentUser = newUser;
        loggedIn = true;

        // Продолжение содержания файла src/application/processors/authProcessor.cpp
        logger->info(QString("Зарегистрирован новый пользователь: %1 (ID: %2) с ролью %3")
                         .arg(newUser.getUsername())
                         .arg(newUser.getId())
                         .arg(userRoleToString(newUser.getRole())));

        return AuthResult(newUser);
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при регистрации пользователя: %1: %2").arg(email).arg(e.what()));
        return AuthResult(AuthErrorType::RegistrationFailed, QString("Ошибка регистрации: %1").arg(e.what()));
    }
}

bool AuthProcessor::logout() {
    if (loggedIn && currentUser) {
        logger->info(QString("Выход пользователя: %1 (ID: %2)")
                         .arg(currentUser->getUsername())
                         .arg(currentUser->getId()));
    }

    currentUser = std::nullopt;
    loggedIn = false;
    return true;
}

std::optional<User> AuthProcessor::getCurrentUser() const {
    return currentUser;
}

bool AuthProcessor::isUserLoggedIn() const {
    return loggedIn;
}

int AuthProcessor::getCurrentUserId() const {
    if (currentUser) {
        return currentUser->getId();
    }
    return -1;
}

bool AuthProcessor::isCurrentUserAdmin() const {
    if (loggedIn && currentUser) {
        return currentUser->getRole() == UserRole::ADMIN;
    }
    return false;
}

bool AuthProcessor::hasPermission(const QString& permission) const {
    // Если пользователь админ, у него есть любое разрешение
    if (isCurrentUserAdmin()) {
        return true;
    }

    // Базовая система разрешений основана на ролях
    // Для более сложной системы, здесь можно реализовать детальную проверку прав
    if (permission == "user.view" || permission == "recipe.view") {
        // Любой авторизованный пользователь может просматривать пользователей и рецепты
        return loggedIn;
    } else if (permission == "user.edit" || permission == "user.delete") {
        // Редактировать и удалять пользователей может только админ
        return false;
    }

    logger->warn(QString("Запрошено неизвестное разрешение: %1").arg(permission));
    return false;
}
