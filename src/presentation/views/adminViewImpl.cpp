#include "adminViewImpl.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>

AdminViewImpl::AdminViewImpl(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void AdminViewImpl::setupUi() {
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Header with back button and title
    QHBoxLayout* headerLayout = new QHBoxLayout();

    backButton = new QPushButton("< Назад");
    backButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Title
    QLabel* titleLabel = new QLabel("Панель администратора");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignLeft);

    headerLayout->addWidget(backButton);
    headerLayout->addWidget(titleLabel, 1);

    // Add header to main layout
    mainLayout->addLayout(headerLayout);

    // Create main tab widget for admin panel
    mainTabWidget = new QTabWidget();
    
    // Create user management view
    userManagementView = new AdminUserViewImpl();
    
    // Create recipe management view
    recipeManagementView = new AdminRecipeViewImpl();

    // Add tabs to main tab widget
    mainTabWidget->addTab(userManagementView, "Управление пользователями");
    mainTabWidget->addTab(recipeManagementView, "Управление рецептами");

    // Add main tab widget to layout
    mainLayout->addWidget(mainTabWidget);

    // Connect signals
    connect(backButton, &QPushButton::clicked, this, &AdminViewImpl::backToMainRequested);
    
    // Forward signals from user management view
    connect(userManagementView, &AdminUserViewImpl::userSelected, 
            this, &AdminViewImpl::userSelected);
    connect(userManagementView, &AdminUserViewImpl::searchUsersRequested,
            this, &AdminViewImpl::searchUsersRequested);
    connect(userManagementView, &AdminUserViewImpl::deleteUserRequested,
            this, &AdminViewImpl::deleteUserRequested);
    connect(userManagementView, &AdminUserViewImpl::updateUserRoleRequested,
            this, &AdminViewImpl::updateUserRoleRequested);
    connect(userManagementView, &AdminUserViewImpl::refreshUsersRequested,
            this, &AdminViewImpl::refreshUsersRequested);
    connect(userManagementView, &AdminUserViewImpl::filterUsersByRoleRequested,
            this, &AdminViewImpl::filterUsersByRoleRequested);
    connect(userManagementView, &AdminUserViewImpl::updateUserDataRequested,
            this, &AdminViewImpl::updateUserDataRequested);
}

void AdminViewImpl::displayUsers(const QList<User>& users) {
    // Forward to user management view
    userManagementView->displayUsers(users);
}

void AdminViewImpl::displayUserProfile(const UserProfileDTO& profile) {
    // Forward to user management view
    userManagementView->displayUserProfile(profile);
    
    // Switch to user management tab
    mainTabWidget->setCurrentIndex(0);
}

void AdminViewImpl::showUserDeletedStatus(bool success, const QString& message) {
    // Forward to user management view
    userManagementView->showUserDeletedStatus(success, message);
}

void AdminViewImpl::showRoleUpdatedStatus(bool success, const QString& message) {
    // Forward to user management view
    userManagementView->showRoleUpdatedStatus(success, message);
}

void AdminViewImpl::showError(const QString& errorMessage) {
    QMessageBox::critical(this, "Ошибка", errorMessage);
}

void AdminViewImpl::switchToMain() {
    emit backToMainRequested();
}

int AdminViewImpl::getSelectedUserId() const {
    // Forward to user management view
    return userManagementView->getSelectedUserId();
}

UserRole AdminViewImpl::getSelectedRole() const {
    // Forward to user management view
    return userManagementView->getSelectedRole();
}

QString AdminViewImpl::getSearchQuery() const {
    // Forward to user management view
    return userManagementView->getSearchQuery();
}

void AdminViewImpl::showMessage(const QString& message) {
    // Forward to user management view
    userManagementView->showMessage(message);
}

void AdminViewImpl::manageRecipesRequested() {
    // Switch to recipes management tab
    mainTabWidget->setCurrentIndex(1);
}

#include "moc_adminViewImpl.cpp"