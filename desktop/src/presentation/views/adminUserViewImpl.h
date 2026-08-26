#ifndef ADMINUSERVIEWIMPL_H
#define ADMINUSERVIEWIMPL_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTableWidget>
#include <QTabWidget>
#include "adminUserView.h"

class AdminUserViewImpl : public QWidget, public AdminUserView {
    Q_OBJECT

public:
    explicit AdminUserViewImpl(QWidget* parent = nullptr);
    ~AdminUserViewImpl() override = default;

    // Implementation of AdminUserView interface
    void displayUsers(const QList<User>& users) override;
    void displayUserProfile(const UserProfileDTO& profile) override;
    void showUserDeletedStatus(bool success, const QString& message) override;
    void showRoleUpdatedStatus(bool success, const QString& message) override;
    void showError(const QString& errorMessage) override;
    void showMessage(const QString& message) override;
    int getSelectedUserId() const override;
    UserRole getSelectedRole() const override;
    QString getSearchQuery() const override;

signals:
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
    QTabWidget* tabWidget;

    // Users list tab
    QWidget* usersListTab;
    QLineEdit* searchLineEdit;
    QPushButton* searchButton;
    QComboBox* roleFilterComboBox;
    QTableWidget* usersTableWidget;
    QPushButton* refreshButton;

    // User details tab
    QWidget* userDetailsTab;
    QLabel* usernameLabel;
    QLabel* emailLabel;
    QLabel* roleLabel;
    QLabel* registrationDateLabel;
    QLabel* favoritesCountLabel;
    QLabel* reviewsCountLabel;
    QComboBox* newRoleComboBox;
    QPushButton* updateRoleButton;
    QPushButton* deleteUserButton;
    QPushButton* backToListButton;

    QLineEdit* editUsernameLineEdit;
    QLineEdit* editEmailLineEdit;
    QLineEdit* editPasswordLineEdit;
    QLineEdit* editConfirmPasswordLineEdit;
    QPushButton* updateUserDataButton;

    int selectedUserId;

    void setupUi();
    void setupUsersListTab();
    void setupUserDetailsTab();
    void clearUserDetails();
    void populateRoleComboBox(QComboBox* comboBox);
    void updateUserActions(UserRole currentRole);
    QString roleToDisplayString(UserRole role);
};

#endif // ADMINUSERVIEWIMPL_H