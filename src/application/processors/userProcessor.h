#ifndef USERPROCESSOR_H
#define USERPROCESSOR_H

#include "application/interfaces/processors/IUserProcessor.h"
#include "infrastructure/repositories/IUserRepository.h"
#include "infrastructure/repositories/IFavoriteRepository.h"
#include "infrastructure/repositories/IReviewRepository.h"
#include "utils/logging/ILogger.h"
#include <memory>

class UserProcessor : public IUserProcessor {
public:
    explicit UserProcessor(
        std::shared_ptr<IUserRepository> userRepository,
        std::shared_ptr<IFavoriteRepository> favoriteRepository,
        std::shared_ptr<IReviewRepository> reviewRepository,
        std::shared_ptr<ILogger> logger = nullptr);

    ~UserProcessor() override = default;

    // Получение информации о пользователе
    std::optional<User> getUserById(int userId) override;
    std::optional<User> getUserByEmail(const QString& email) override;
    QList<User> getUsersByUsername(const QString& username) override;

    // Получение пользователей по роли и всех пользователей
    QList<User> getUsersByRole(UserRole role) override;
    QList<User> getAllUsers(int limit = 100, int offset = 0) override;

    // Управление профилем пользователя
    bool updateUserProfile(const User& user) override;
    bool updateUserRole(int userId, UserRole role) override;
    bool deleteUser(int userId) override;

    // Бизнес-логика с профилем
    std::optional<UserProfileDTO> getUserProfile(int userId) override;

private:
    std::shared_ptr<IUserRepository> userRepository;
    std::shared_ptr<IFavoriteRepository> favoriteRepository;
    std::shared_ptr<IReviewRepository> reviewRepository;
    std::shared_ptr<ILogger> logger;
};

#endif // USERPROCESSOR_H
