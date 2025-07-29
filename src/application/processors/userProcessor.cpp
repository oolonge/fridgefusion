#include "userProcessor.h"
#include <QDebug>
#include "utils/logging/LoggerService.h"

UserProcessor::UserProcessor(
    std::shared_ptr<IUserRepository> userRepository,
    std::shared_ptr<IFavoriteRepository> favoriteRepository,
    std::shared_ptr<IReviewRepository> reviewRepository,
    std::shared_ptr<ILogger> logger)
    : userRepository(std::move(userRepository)),
    favoriteRepository(std::move(favoriteRepository)),
    reviewRepository(std::move(reviewRepository)),
    logger(logger ? logger : LoggerService::getInstance().getLogger("default")) {
}

std::optional<User> UserProcessor::getUserById(int userId) {
    try {
        return userRepository->getUserById(userId);
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении пользователя по ID %1: %2").arg(userId).arg(e.what()));
        return std::nullopt;
    }
}

std::optional<User> UserProcessor::getUserByEmail(const QString& email) {
    try {
        return userRepository->getUserByEmail(email);
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении пользователя по email %1: %2").arg(email).arg(e.what()));
        return std::nullopt;
    }
}

QList<User> UserProcessor::getUsersByUsername(const QString& username) {
    try {
        return userRepository->getUsersByUsername(username);
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении пользователей по имени %1: %2").arg(username).arg(e.what()));
        return QList<User>();
    }
}

QList<User> UserProcessor::getUsersByRole(UserRole role) {
    try {
        return userRepository->getUsersByRole(role);
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении пользователей по роли %1: %2")
                          .arg(userRoleToString(role)).arg(e.what()));
        return QList<User>();
    }
}

QList<User> UserProcessor::getAllUsers(int limit, int offset) {
    try {
        return userRepository->getAllUsers(limit, offset);
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении всех пользователей: %1").arg(e.what()));
        return QList<User>();
    }
}

bool UserProcessor::updateUserProfile(const User& user) {
    try {
        // Проверяем существование пользователя перед обновлением
        auto existingUser = userRepository->getUserById(user.getId());
        if (!existingUser) {
            logger->warn(QString("Попытка обновить несуществующего пользователя с ID %1").arg(user.getId()));
            return false;
        }

        // Проверяем, что email не занят другим пользователем (если он изменился)
        if (existingUser->getEmail() != user.getEmail()) {
            auto userWithSameEmail = userRepository->getUserByEmail(user.getEmail());
            if (userWithSameEmail && userWithSameEmail->getId() != user.getId()) {
                logger->warn(QString("Email %1 уже используется другим пользователем").arg(user.getEmail()));
                return false;
            }
        }

        // Обновляем профиль пользователя
        bool success = userRepository->updateUser(user);

        if (success) {
            logger->info(QString("Обновлен профиль пользователя с ID %1").arg(user.getId()));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при обновлении профиля пользователя: %1").arg(e.what()));
        return false;
    }
}

bool UserProcessor::updateUserRole(int userId, UserRole role) {
    try {
        // Проверяем существование пользователя
        auto existingUser = userRepository->getUserById(userId);
        if (!existingUser) {
            logger->warn(QString("Попытка обновить роль несуществующего пользователя с ID %1").arg(userId));
            return false;
        }

        // Логируем изменение роли
        logger->info(QString("Изменение роли пользователя с ID %1 с %2 на %3")
                         .arg(userId)
                         .arg(userRoleToString(existingUser->getRole()))
                         .arg(userRoleToString(role)));

        // Обновляем роль пользователя
        return userRepository->updateUserRole(userId, role);
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при обновлении роли пользователя: %1").arg(e.what()));
        return false;
    }
}

bool UserProcessor::deleteUser(int userId) {
    try {
        // Проверяем существование пользователя перед удалением
        auto existingUser = userRepository->getUserById(userId);
        if (!existingUser) {
            logger->warn(QString("Попытка удалить несуществующего пользователя с ID %1").arg(userId));
            return false;
        }

        logger->info(QString("Удаление пользователя с ID %1 и именем %2")
                         .arg(userId)
                         .arg(existingUser->getUsername()));

        // Удаляем пользователя
        return userRepository->deleteUser(userId);
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при удалении пользователя: %1").arg(e.what()));
        return false;
    }
}

std::optional<UserProfileDTO> UserProcessor::getUserProfile(int userId) {
    try {
        // Получаем основную информацию о пользователе
        auto user = userRepository->getUserById(userId);
        if (!user) {
            logger->warn(QString("Попытка получить профиль несуществующего пользователя с ID %1").arg(userId));
            return std::nullopt;
        }

        // Создаем DTO на основе сущности пользователя
        UserProfileDTO profileDTO(*user);

        // Дополняем информацией о количестве избранных рецептов
        auto favorites = favoriteRepository->getFavoritesByUserId(userId);
        profileDTO.favoriteRecipesCount = favorites.size();

        // Дополняем информацией о количестве отзывов
        auto reviews = reviewRepository->getReviewsByUserId(userId);
        profileDTO.reviewsCount = reviews.size();

        logger->debug(QString("Получен профиль пользователя с ID %1: избранных рецептов - %2, отзывов - %3")
                          .arg(userId)
                          .arg(profileDTO.favoriteRecipesCount)
                          .arg(profileDTO.reviewsCount));

        return profileDTO;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении профиля пользователя: %1").arg(e.what()));
        return std::nullopt;
    }
}
