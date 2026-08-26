#ifndef ADMINVIEWIMPL_H
#define ADMINVIEWIMPL_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include "adminView.h"
#include "adminUserViewImpl.h"
#include "adminRecipeViewImpl.h"

class AdminViewImpl : public QWidget, public AdminView {
    Q_OBJECT

public:
    explicit AdminViewImpl(QWidget* parent = nullptr);
    ~AdminViewImpl() override = default;

    // Implementation of AdminView
    void displayUsers(const QList<User>& users) override;
    void displayUserProfile(const UserProfileDTO& profile) override;
    void showUserDeletedStatus(bool success, const QString& message) override;
    void showRoleUpdatedStatus(bool success, const QString& message) override;
    void showError(const QString& errorMessage) override;
    void switchToMain() override;
    void showMessage(const QString& message) override;
    int getSelectedUserId() const override;
    UserRole getSelectedRole() const override;
    QString getSearchQuery() const override;
    void manageRecipesRequested() override;

signals:
    void backToMainRequested();
    void userSelected(int userId);
    void searchUsersRequested();
    void deleteUserRequested(int userId);
    void updateUserRoleRequested(int userId, UserRole role);
    void refreshUsersRequested();
    void filterUsersByRoleRequested(UserRole role);
    void updateUserDataRequested(int userId, const QString& username, const QString& email,
                               const QString& password, const QString& confirmPassword);

private:
    // UI elements
    QTabWidget* mainTabWidget;
    QPushButton* backButton;
    
    // User management tab
    AdminUserViewImpl* userManagementView;
    
    // Recipe management tab
    AdminRecipeViewImpl* recipeManagementView;

    void setupUi();
};

#endif // ADMINVIEWIMPL_H