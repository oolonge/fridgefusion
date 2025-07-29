#ifndef AUTHRESULT_H
#define AUTHRESULT_H

#include "domain/entities/user.h"
#include <QString>
#include <optional>

enum class AuthErrorType {
    None,
    UserNotFound,
    InvalidPassword,
    RegistrationFailed,
    EmailAlreadyExists,
    InvalidInput
};

class AuthResult {
public:
    // Constructor for successful authentication
    explicit AuthResult(const User& user)
        : success(true), user(user), errorType(AuthErrorType::None) {}

    // Constructor for failed authentication
    explicit AuthResult(AuthErrorType errorType, const QString& errorMessage = QString())
        : success(false), errorType(errorType), errorMessage(errorMessage) {}

    // Success status
    bool isSuccess() const { return success; }

    // Get authenticated user (if successful)
    std::optional<User> getUser() const {
        if (success) {
            return user;
        }
        return std::nullopt;
    }

    // Get error details (if failed)
    AuthErrorType getErrorType() const { return errorType; }
    QString getErrorMessage() const { return errorMessage; }

private:
    bool success;
    std::optional<User> user;
    AuthErrorType errorType;
    QString errorMessage;
};

#endif // AUTHRESULT_H
