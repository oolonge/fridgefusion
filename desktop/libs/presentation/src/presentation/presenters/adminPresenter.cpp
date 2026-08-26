#include "adminPresenter.h"
#include <QDebug>

AdminPresenter::AdminPresenter(
    std::shared_ptr<IAdminProcessor> adminProcessor,
    std::shared_ptr<IAuthProcessor> authProcessor,
    std::shared_ptr<IUserProcessor> userProcessor,
    std::shared_ptr<IPasswordHasher> passwordHasher,
    AdminView* view)
    : adminProcessor(std::move(adminProcessor)),
    authProcessor(std::move(authProcessor)),
    userProcessor(std::move(userProcessor)),
    passwordHasher(std::move(passwordHasher)),
    view(view) {
}

void AdminPresenter::loadAllUsers(int limit, int offset) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        QList<User> users = adminProcessor->getAllUsers(limit, offset);
        view->displayUsers(users);
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при загрузке списка пользователей: %1").arg(e.what()));
    }
}

void AdminPresenter::searchUsers(const QString& searchTerm) {
    if (!checkAdminAccess()) {
        return;
    }

    if (searchTerm.isEmpty()) {
        loadAllUsers();
        return;
    }

    try {
        QList<User> users = adminProcessor->searchUsers(searchTerm);
        view->displayUsers(users);
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при поиске пользователей: %1").arg(e.what()));
    }
}

void AdminPresenter::filterUsersByRole(UserRole role) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        QList<User> users = adminProcessor->getUsersByRole(role);
        view->displayUsers(users);
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при фильтрации пользователей по роли: %1").arg(e.what()));
    }
}

void AdminPresenter::loadUserProfile(int userId) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        auto profile = adminProcessor->getUserProfile(userId);
        if (profile) {
            view->displayUserProfile(*profile);
        } else {
            view->showError(QString("Пользователь с ID %1 не найден").arg(userId));
        }
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при загрузке профиля пользователя: %1").arg(e.what()));
    }
}

void AdminPresenter::updateUserRole(int userId, UserRole newRole) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        // Получаем текущую информацию о пользователе для лога
        auto profile = adminProcessor->getUserProfile(userId);
        if (!profile) {
            view->showError(QString("Пользователь с ID %1 не найден").arg(userId));
            return;
        }

        // Проверка: не пытаемся ли мы изменить роль основного админа
        if (userId == 1 && profile->role == UserRole::ADMIN) {
            view->showRoleUpdatedStatus(
                false,
                "Невозможно изменить роль главного администратора системы"
                );
            return;
        }

        // Обновляем роль
        bool success = adminProcessor->updateUserRole(userId, newRole);

        if (success) {
            view->showRoleUpdatedStatus(
                true,
                QString("Роль пользователя %1 успешно изменена с %2 на %3")
                    .arg(profile->username)
                    .arg(userRoleToString(profile->role))
                    .arg(userRoleToString(newRole))
                );
        } else {
            view->showRoleUpdatedStatus(
                false,
                QString("Не удалось изменить роль пользователя %1").arg(profile->username)
                );
        }
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при обновлении роли пользователя: %1").arg(e.what()));
    }
}

void AdminPresenter::deleteUser(int userId) {
    if (!checkAdminAccess()) {
        return;
    }

    try {
        // Получаем информацию о пользователе перед удалением
        auto profile = adminProcessor->getUserProfile(userId);
        if (!profile) {
            view->showUserDeletedStatus(false, QString("Пользователь с ID %1 не найден").arg(userId));
            return;
        }

        // Проверка: не пытаемся ли мы удалить основного админа
        if (userId == 1 && profile->role == UserRole::ADMIN) {
            view->showUserDeletedStatus(
                false,
                "Невозможно удалить главного администратора системы"
                );
            return;
        }

        // Удаляем пользователя
        bool success = adminProcessor->deleteUser(userId);

        if (success) {
            view->showUserDeletedStatus(
                true,
                QString("Пользователь %1 успешно удален").arg(profile->username)
                );
        } else {
            view->showUserDeletedStatus(
                false,
                QString("Не удалось удалить пользователя %1").arg(profile->username)
                );
        }
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при удалении пользователя: %1").arg(e.what()));
    }
}

bool AdminPresenter::checkAdminAccess() {
    if (!authProcessor->isUserLoggedIn()) {
        view->showError("Необходимо войти в систему для доступа к администрированию");
        view->switchToMain();
        return false;
    }

    if (!authProcessor->isCurrentUserAdmin()) {
        view->showError("У вас нет прав администратора для выполнения этой операции");
        view->switchToMain();
        return false;
    }

    return true;
}

void AdminPresenter::updateUserData(int userId, const QString& username, const QString& email,
                                    const QString& password, const QString& confirmPassword) {
    if (!checkAdminAccess()) {
        return;
    }

    // Проверка наличия данных
    if (username.isEmpty() || email.isEmpty()) {
        view->showError("Имя пользователя и Email не могут быть пустыми");
        return;
    }

    // Проверка совпадения паролей
    if (!password.isEmpty() && password != confirmPassword) {
        view->showError("Пароли не совпадают");
        return;
    }

    try {
        // Получаем текущие данные пользователя
        auto userOpt = userProcessor->getUserById(userId);
        if (!userOpt) {
            view->showError(QString("Пользователь с ID %1 не найден").arg(userId));
            return;
        }

        User updatedUser = *userOpt;

        // Обновляем данные
        updatedUser.setUsername(username);
        updatedUser.setEmail(email);

        // Обновляем пароль, если он был указан
        if (!password.isEmpty()) {
            QString hashedPassword = passwordHasher->hashPassword(password);
            updatedUser.setPasswordHash(hashedPassword);
        }

        // Выполняем обновление
        bool success = userProcessor->updateUserProfile(updatedUser);

        if (success) {
            view->showMessage("Данные пользователя успешно обновлены");

            // Перезагружаем профиль пользователя
            loadUserProfile(userId);
        } else {
            view->showError("Не удалось обновить данные пользователя");
        }
    } catch (const std::exception& e) {
        view->showError(QString("Ошибка при обновлении данных пользователя: %1").arg(e.what()));
    }
}
