#ifndef AUTHPROCESSOR_H
#define AUTHPROCESSOR_H

#include "application/interfaces/processors/IAuthProcessor.h"
#include "application/interfaces/services/IPasswordHasher.h"
#include "infrastructure/repositories/IUserRepository.h"
#include "utils/logging/ILogger.h"
#include <memory>

class AuthProcessor : public IAuthProcessor {
public:
    AuthProcessor(
        std::shared_ptr<IUserRepository> userRepository,
        std::shared_ptr<IPasswordHasher> passwordHasher,
        std::shared_ptr<ILogger> logger = nullptr
        );
    ~AuthProcessor() override = default;

    // Authentication
    AuthResult login(const QString& email, const QString& password) override;

    // Registration
    AuthResult registerUser(const QString& username, const QString& email, const QString& password, UserRole role = UserRole::USER) override;

    // Session management
    bool logout() override;
    std::optional<User> getCurrentUser() const override;
    bool isUserLoggedIn() const override;
    int getCurrentUserId() const override;

    // Permission checks
    bool isCurrentUserAdmin() const override;
    bool hasPermission(const QString& permission) const override;

private:
    std::shared_ptr<IUserRepository> userRepository;
    std::shared_ptr<IPasswordHasher> passwordHasher;
    std::shared_ptr<ILogger> logger;

    // Current user state
    std::optional<User> currentUser;
    bool loggedIn;
};

#endif // AUTHPROCESSOR_H
