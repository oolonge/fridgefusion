#ifndef ADMINPRESENTER_H
#define ADMINPRESENTER_H

#include "application/interfaces/processors/IAdminProcessor.h"
#include "application/interfaces/processors/IAuthProcessor.h"
#include "application/interfaces/processors/IUserProcessor.h"
#include "application/interfaces/services/IPasswordHasher.h"
#include "presentation/views/adminView.h"
#include <memory>

class AdminPresenter {
public:
    AdminPresenter(
        std::shared_ptr<IAdminProcessor> adminProcessor,
        std::shared_ptr<IAuthProcessor> authProcessor,
        std::shared_ptr<IUserProcessor> userProcessor,
        std::shared_ptr<IPasswordHasher> passwordHasher,
        AdminView* view);
    ~AdminPresenter() = default;

    void loadAllUsers(int limit = 100, int offset = 0);
    void searchUsers(const QString& searchTerm);
    void filterUsersByRole(UserRole role);
    void loadUserProfile(int userId);
    void updateUserRole(int userId, UserRole newRole);
    void deleteUser(int userId);
    bool checkAdminAccess();
    void updateUserData(int userId, const QString& username, const QString& email,
                        const QString& password, const QString& confirmPassword);

private:
    std::shared_ptr<IAdminProcessor> adminProcessor;
    std::shared_ptr<IAuthProcessor> authProcessor;
    std::shared_ptr<IUserProcessor> userProcessor;
    std::shared_ptr<IPasswordHasher> passwordHasher;
    AdminView* view;
};

#endif // ADMINPRESENTER_H
