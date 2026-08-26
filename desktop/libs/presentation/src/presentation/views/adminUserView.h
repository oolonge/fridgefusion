#ifndef ADMINUSERVIEW_H
#define ADMINUSERVIEW_H

#include <QList>
#include "domain/entities/user.h"
#include "domain/dtos/userProfileDTO.h"

class AdminUserView {
public:
    virtual ~AdminUserView() = default;

    // Display methods
    virtual void displayUsers(const QList<User>& users) = 0;
    virtual void displayUserProfile(const UserProfileDTO& profile) = 0;
    
    // Status notification methods
    virtual void showUserDeletedStatus(bool success, const QString& message) = 0;
    virtual void showRoleUpdatedStatus(bool success, const QString& message) = 0;
    virtual void showError(const QString& errorMessage) = 0;
    virtual void showMessage(const QString& message) = 0;
    
    // Data access methods
    virtual int getSelectedUserId() const = 0;
    virtual UserRole getSelectedRole() const = 0;
    virtual QString getSearchQuery() const = 0;
};

#endif // ADMINUSERVIEW_H