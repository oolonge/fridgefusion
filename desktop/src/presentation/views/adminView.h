#ifndef ADMINVIEW_H
#define ADMINVIEW_H

#include <QList>
#include "domain/entities/user.h"
#include "domain/dtos/userProfileDTO.h"

class AdminView {
public:
    virtual ~AdminView() = default;

    virtual void displayUsers(const QList<User>& users) = 0;

    virtual void displayUserProfile(const UserProfileDTO& profile) = 0;

    virtual void showUserDeletedStatus(bool success, const QString& message) = 0;
    virtual void showRoleUpdatedStatus(bool success, const QString& message) = 0;

    virtual void showError(const QString& errorMessage) = 0;

    virtual void switchToMain() = 0;

    virtual void showMessage(const QString& message) = 0;

    virtual int getSelectedUserId() const = 0;
    virtual UserRole getSelectedRole() const = 0;
    virtual QString getSearchQuery() const = 0;
    
    // Навигация к управлению рецептами
    virtual void manageRecipesRequested() = 0;
};

#endif // ADMINVIEW_H
