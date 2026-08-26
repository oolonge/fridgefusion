#include "adminUserViewImpl.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QGroupBox>
#include <QSpacerItem>
#include <QDebug>

AdminUserViewImpl::AdminUserViewImpl(QWidget* parent)
    : QWidget(parent), selectedUserId(-1)
{
    setupUi();
}

void AdminUserViewImpl::setupUi() {
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    // Create TabWidget for user management
    tabWidget = new QTabWidget();

    // Setup tabs
    setupUsersListTab();
    setupUserDetailsTab();

    // Add tabs to TabWidget
    tabWidget->addTab(usersListTab, "Список пользователей");
    tabWidget->addTab(userDetailsTab, "Детали пользователя");

    // By default, user details tab is disabled
    tabWidget->setTabEnabled(1, false);

    // Add TabWidget to main layout
    mainLayout->addWidget(tabWidget);

    // Connect signals
    connect(searchButton, &QPushButton::clicked, this, &AdminUserViewImpl::searchUsersRequested);
    connect(searchLineEdit, &QLineEdit::returnPressed, this, &AdminUserViewImpl::searchUsersRequested);

    connect(roleFilterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        if (index == 0) {
            // "All roles"
            emit refreshUsersRequested();
        } else {
            UserRole selectedRole = static_cast<UserRole>(roleFilterComboBox->currentData().toInt());
            emit filterUsersByRoleRequested(selectedRole);
        }
    });

    connect(updateUserDataButton, &QPushButton::clicked, [this]() {
        if (selectedUserId > 0) {
            emit updateUserDataRequested(
                selectedUserId,
                editUsernameLineEdit->text(),
                editEmailLineEdit->text(),
                editPasswordLineEdit->text(),
                editConfirmPasswordLineEdit->text()
                );
        } else {
            QMessageBox::warning(this, "Предупреждение", "Пользователь не выбран");
        }
    });

    connect(usersTableWidget, &QTableWidget::cellDoubleClicked, [this](int row, int) {
        QTableWidgetItem* idItem = usersTableWidget->item(row, 0);
        if (idItem) {
            int userId = idItem->text().toInt();
            selectedUserId = userId;
            emit userSelected(userId);

            // Switch to user details tab and enable it
            tabWidget->setTabEnabled(1, true);
            tabWidget->setCurrentIndex(1);
        }
    });

    connect(refreshButton, &QPushButton::clicked, this, &AdminUserViewImpl::refreshUsersRequested);
    connect(backToListButton, &QPushButton::clicked, [this]() {
        tabWidget->setCurrentIndex(0);
    });

    connect(updateRoleButton, &QPushButton::clicked, [this]() {
        if (selectedUserId > 0) {
            UserRole newRole = getSelectedRole();
            emit updateUserRoleRequested(selectedUserId, newRole);
        } else {
            QMessageBox::warning(this, "Предупреждение", "Сначала выберите пользователя");
        }
    });

    connect(deleteUserButton, &QPushButton::clicked, [this]() {
        if (selectedUserId > 0) {
            // Ask for confirmation
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Подтверждение удаления",
                "Вы уверены, что хотите удалить этого пользователя? Это действие нельзя отменить.",
                QMessageBox::Yes | QMessageBox::No
                );

            if (reply == QMessageBox::Yes) {
                emit deleteUserRequested(selectedUserId);
            }
        } else {
            QMessageBox::warning(this, "Предупреждение", "Сначала выберите пользователя");
        }
    });

    // Clear user details
    clearUserDetails();
}

void AdminUserViewImpl::setupUsersListTab() {
    usersListTab = new QWidget();
    QVBoxLayout* usersListLayout = new QVBoxLayout(usersListTab);

    // Search and filter panel
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLineEdit = new QLineEdit();
    searchLineEdit->setPlaceholderText("Поиск пользователей...");
    searchButton = new QPushButton("Поиск");

    QLabel* filterLabel = new QLabel("Фильтр по роли:");
    roleFilterComboBox = new QComboBox();
    populateRoleComboBox(roleFilterComboBox);
    roleFilterComboBox->insertItem(0, "Все роли", QVariant());
    roleFilterComboBox->setCurrentIndex(0);

    searchLayout->addWidget(searchLineEdit);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(filterLabel);
    searchLayout->addWidget(roleFilterComboBox);

    // Users table
    usersTableWidget = new QTableWidget();
    usersTableWidget->setColumnCount(4);
    QStringList headers = {"ID", "Имя пользователя", "Email", "Роль"};
    usersTableWidget->setHorizontalHeaderLabels(headers);
    usersTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    usersTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    usersTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    usersTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    usersTableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Buttons panel
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    refreshButton = new QPushButton("Обновить список");

    buttonsLayout->addWidget(refreshButton);
    buttonsLayout->addStretch();

    // Assemble users list tab
    usersListLayout->addLayout(searchLayout);
    usersListLayout->addWidget(usersTableWidget);
    usersListLayout->addLayout(buttonsLayout);
}

void AdminUserViewImpl::setupUserDetailsTab() {
    userDetailsTab = new QWidget();
    QVBoxLayout* userDetailsLayout = new QVBoxLayout(userDetailsTab);

    // Создаем основной grid layout для двух колонок
    QGridLayout* mainGridLayout = new QGridLayout();

    // Left column - user information
    QVBoxLayout* leftColumnLayout = new QVBoxLayout();

    // User information
    QGroupBox* userInfoGroup = new QGroupBox("Информация о пользователе");
    QFormLayout* userInfoLayout = new QFormLayout(userInfoGroup);

    usernameLabel = new QLabel();
    emailLabel = new QLabel();
    roleLabel = new QLabel();
    registrationDateLabel = new QLabel();
    favoritesCountLabel = new QLabel();
    reviewsCountLabel = new QLabel();

    userInfoLayout->addRow("Имя пользователя:", usernameLabel);
    userInfoLayout->addRow("Email:", emailLabel);
    userInfoLayout->addRow("Роль:", roleLabel);
    userInfoLayout->addRow("Дата регистрации:", registrationDateLabel);
    userInfoLayout->addRow("Избранные рецепты:", favoritesCountLabel);
    userInfoLayout->addRow("Отзывы:", reviewsCountLabel);

    leftColumnLayout->addWidget(userInfoGroup);
    leftColumnLayout->addStretch();

    // Right column - edit form
    QVBoxLayout* rightColumnLayout = new QVBoxLayout();

    // Edit data group
    QGroupBox* editGroup = new QGroupBox("Редактирование данных");
    QFormLayout* editLayout = new QFormLayout(editGroup);

    editUsernameLineEdit = new QLineEdit();
    editEmailLineEdit = new QLineEdit();
    editPasswordLineEdit = new QLineEdit();
    editPasswordLineEdit->setEchoMode(QLineEdit::Password);
    editConfirmPasswordLineEdit = new QLineEdit();
    editConfirmPasswordLineEdit->setEchoMode(QLineEdit::Password);

    editLayout->addRow("Новое имя пользователя:", editUsernameLineEdit);
    editLayout->addRow("Новый Email:", editEmailLineEdit);
    editLayout->addRow("Новый пароль:", editPasswordLineEdit);
    editLayout->addRow("Подтверждение пароля:", editConfirmPasswordLineEdit);

    // Update button
    updateUserDataButton = new QPushButton("Обновить данные");
    editLayout->addRow("", updateUserDataButton);

    rightColumnLayout->addWidget(editGroup);

    // Create actions group
    QGroupBox* actionsGroup = new QGroupBox("Действия");
    QVBoxLayout* actionsLayout = new QVBoxLayout(actionsGroup);

    // Role change
    QHBoxLayout* roleChangeLayout = new QHBoxLayout();
    QLabel* newRoleLabel = new QLabel("Новая роль:");
    newRoleComboBox = new QComboBox();
    populateRoleComboBox(newRoleComboBox);
    updateRoleButton = new QPushButton("Изменить роль");

    roleChangeLayout->addWidget(newRoleLabel);
    roleChangeLayout->addWidget(newRoleComboBox);
    roleChangeLayout->addWidget(updateRoleButton);

    // Delete user
    deleteUserButton = new QPushButton("Удалить пользователя");
    deleteUserButton->setStyleSheet("background-color: #f44336; color: white;");

    actionsLayout->addLayout(roleChangeLayout);
    actionsLayout->addWidget(deleteUserButton);

    rightColumnLayout->addWidget(actionsGroup);
    rightColumnLayout->addStretch();

    // Добавляем колонки в основной grid layout
    mainGridLayout->addLayout(leftColumnLayout, 0, 0);
    mainGridLayout->addLayout(rightColumnLayout, 0, 1);
    mainGridLayout->setColumnStretch(0, 1);
    mainGridLayout->setColumnStretch(1, 1);

    userDetailsLayout->addLayout(mainGridLayout);

    // Back to list button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    backToListButton = new QPushButton("Вернуться к списку");
    buttonLayout->addWidget(backToListButton);
    buttonLayout->addStretch();

    // Add button to main layout
    userDetailsLayout->addLayout(buttonLayout);
}

void AdminUserViewImpl::displayUsers(const QList<User>& users) {
    usersTableWidget->setRowCount(0); // Clear the table

    for (int i = 0; i < users.size(); ++i) {
        const User& user = users[i];

        usersTableWidget->insertRow(i);

        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(user.getId()));
        QTableWidgetItem* usernameItem = new QTableWidgetItem(user.getUsername());
        QTableWidgetItem* emailItem = new QTableWidgetItem(user.getEmail());
        QTableWidgetItem* roleItem = new QTableWidgetItem(roleToDisplayString(user.getRole()));

        usersTableWidget->setItem(i, 0, idItem);
        usersTableWidget->setItem(i, 1, usernameItem);
        usersTableWidget->setItem(i, 2, emailItem);
        usersTableWidget->setItem(i, 3, roleItem);
    }

    // If no users, show a message
    if (users.isEmpty()) {
        usersTableWidget->setRowCount(1);
        QTableWidgetItem* noDataItem = new QTableWidgetItem("Нет данных для отображения");
        noDataItem->setTextAlignment(Qt::AlignCenter);
        usersTableWidget->setSpan(0, 0, 1, 4);
        usersTableWidget->setItem(0, 0, noDataItem);
    }
}

void AdminUserViewImpl::displayUserProfile(const UserProfileDTO& profile) {
    selectedUserId = profile.id;

    // Fill user information
    usernameLabel->setText(profile.username);
    emailLabel->setText(profile.email);
    roleLabel->setText(roleToDisplayString(profile.role));
    registrationDateLabel->setText(profile.registrationDate.toString("dd.MM.yyyy hh:mm"));
    favoritesCountLabel->setText(QString::number(profile.favoriteRecipesCount));
    reviewsCountLabel->setText(QString::number(profile.reviewsCount));

    // Pre-fill edit fields with current values
    editUsernameLineEdit->setText(profile.username);
    editEmailLineEdit->setText(profile.email);
    editPasswordLineEdit->clear();  // Don't pre-fill password
    editConfirmPasswordLineEdit->clear();

    // Set current role in combobox
    int roleIndex = newRoleComboBox->findData(static_cast<int>(profile.role));
    if (roleIndex != -1) {
        newRoleComboBox->setCurrentIndex(roleIndex);
    }

    // Update actions availability based on role
    updateUserActions(profile.role);

    // Switch to details tab and enable it
    tabWidget->setTabEnabled(1, true);
    tabWidget->setCurrentIndex(1);
}

void AdminUserViewImpl::showUserDeletedStatus(bool success, const QString& message) {
    if (success) {
        QMessageBox::information(this, "Удаление пользователя", message);
        // Return to users list
        tabWidget->setCurrentIndex(0);
        tabWidget->setTabEnabled(1, false); // Disable details tab
        clearUserDetails();
        selectedUserId = -1;

        // Update users list
        emit refreshUsersRequested();
    } else {
        QMessageBox::warning(this, "Ошибка удаления", message);
    }
}

void AdminUserViewImpl::showRoleUpdatedStatus(bool success, const QString& message) {
    if (success) {
        QMessageBox::information(this, "Изменение роли", message);
        // Update user information
        emit userSelected(selectedUserId);
    } else {
        QMessageBox::warning(this, "Ошибка изменения роли", message);
    }
}

void AdminUserViewImpl::showError(const QString& errorMessage) {
    QMessageBox::critical(this, "Ошибка", errorMessage);
}

void AdminUserViewImpl::showMessage(const QString& message) {
    QMessageBox::information(this, "Информация", message);
}

int AdminUserViewImpl::getSelectedUserId() const {
    return selectedUserId;
}

UserRole AdminUserViewImpl::getSelectedRole() const {
    return static_cast<UserRole>(newRoleComboBox->currentData().toInt());
}

QString AdminUserViewImpl::getSearchQuery() const {
    return searchLineEdit->text();
}

void AdminUserViewImpl::clearUserDetails() {
    usernameLabel->clear();
    emailLabel->clear();
    roleLabel->clear();
    registrationDateLabel->clear();
    favoritesCountLabel->clear();
    reviewsCountLabel->clear();

    // Clear edit fields
    editUsernameLineEdit->clear();
    editEmailLineEdit->clear();
    editPasswordLineEdit->clear();
    editConfirmPasswordLineEdit->clear();
}

void AdminUserViewImpl::populateRoleComboBox(QComboBox* comboBox) {
    comboBox->clear();
    comboBox->addItem("Пользователь", static_cast<int>(UserRole::USER));
    comboBox->addItem("Администратор", static_cast<int>(UserRole::ADMIN));
}

void AdminUserViewImpl::updateUserActions(UserRole currentRole) {
    // Prevent changing the role of the main admin
    bool isMainAdmin = (selectedUserId == 1 && currentRole == UserRole::ADMIN);

    updateRoleButton->setEnabled(!isMainAdmin);
    deleteUserButton->setEnabled(!isMainAdmin);

    if (isMainAdmin) {
        QLabel* warningLabel = new QLabel("Невозможно изменить главного администратора");
        warningLabel->setStyleSheet("color: red");
        // TODO: Add this label to the form if needed
    }
}

QString AdminUserViewImpl::roleToDisplayString(UserRole role) {
    switch (role) {
    case UserRole::ADMIN: return "Администратор";
    case UserRole::USER: return "Пользователь";
    case UserRole::GUEST: return "Гость";
    default: return "Неизвестная роль";
    }
}

#include "moc_adminUserViewImpl.cpp"