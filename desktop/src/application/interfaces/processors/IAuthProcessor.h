#ifndef IAUTHPROCESSOR_H
#define IAUTHPROCESSOR_H

#include "domain/entities/user.h"
#include "domain/dtos/authResult.h"
#include <QString>
#include <optional>

class IAuthProcessor {
public:
    virtual ~IAuthProcessor() = default;

    // Authentication
    virtual AuthResult login(const QString& email, const QString& password) = 0;

    // Registration
    virtual AuthResult registerUser(const QString& username, const QString& email, const QString& password, UserRole role = UserRole::USER) = 0;

    // Session management
    virtual bool logout() = 0;
    virtual std::optional<User> getCurrentUser() const = 0;
    virtual bool isUserLoggedIn() const = 0;
    virtual int getCurrentUserId() const = 0;

    // Permission checks
    virtual bool isCurrentUserAdmin() const = 0;
    virtual bool hasPermission(const QString& permission) const = 0;
};

#endif // IAUTHPROCESSOR_H
