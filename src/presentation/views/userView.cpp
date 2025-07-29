#include "userView.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QDateEdit>
#include <QMouseEvent>

// Добавляем новый класс для карточки избранного рецепта
class FavoriteRecipeCard : public QFrame {
    Q_OBJECT

public:
    explicit FavoriteRecipeCard(int recipeId, QWidget* parent = nullptr)
        : QFrame(parent), m_recipeId(recipeId)
    {
        setFrameShape(QFrame::StyledPanel);
        setLineWidth(1);
        setCursor(Qt::PointingHandCursor);
    }

    int getRecipeId() const { return m_recipeId; }

signals:
    void clicked(int recipeId);

protected:
    void mousePressEvent(QMouseEvent* event) override {
        emit clicked(m_recipeId);
        QFrame::mousePressEvent(event);
    }

private:
    int m_recipeId;
};

class ClickableFrame : public QFrame {
    Q_OBJECT

public:
    explicit ClickableFrame(int recipeId, QWidget* parent = nullptr)
        : QFrame(parent), m_recipeId(recipeId) {
        setCursor(Qt::PointingHandCursor);
    }

    int getRecipeId() const { return m_recipeId; }

signals:
    void clicked(int recipeId);

protected:
    void mousePressEvent(QMouseEvent* event) override {
        emit clicked(m_recipeId);
        QFrame::mousePressEvent(event);
    }

private:
    int m_recipeId;
};

class UserViewImpl : public QWidget, public UserView {
    Q_OBJECT

public:
    explicit UserViewImpl(QWidget* parent = nullptr);
    ~UserViewImpl() override = default;

    // Реализация интерфейса UserView
    void displayUserProfile(const UserProfileDTO& profile) override;
    void displayFavoriteRecipes(const QList<RecipePreviewDTO>& favorites) override;
    void showProfileUpdateSuccess() override;
    void showProfileUpdateError(const QString& errorMessage) override;
    QString getNewUsername() override;
    QString getNewEmail() override;
    QString getCurrentPassword() override;
    QString getNewPassword() override;
    void switchToMainView() override;
    void showRecipeDetails(int recipeId) override;
    void requestLogout() override;

signals:
    void backToMainRequested();
    void profileUpdateRequested();
    void favoriteRecipeSelected(int recipeId);
    void recipeSelected(int recipeId);
    void removeFromFavoritesRequested(int recipeId);
    void logoutRequested();

private:
    // UI элементы
    QPushButton* backButton;
    QLabel* usernameLabel;
    QLabel* emailLabel;
    QLabel* registrationDateLabel;
    QLabel* favoritesCountLabel;
    QLabel* reviewsCountLabel;
    QLineEdit* newUsernameLineEdit;
    QLineEdit* newEmailLineEdit;
    QLineEdit* currentPasswordLineEdit;
    QLineEdit* newPasswordLineEdit;
    QPushButton* updateProfileButton;
    QScrollArea* favoritesScrollArea;
    QWidget* favoritesContainer;
    QVBoxLayout* favoritesLayout;
    QPushButton* logoutButton;

    void setupUi();
    void clearFavoritesList();
    QWidget* createFavoriteRecipeCard(const RecipePreviewDTO& recipe);
};

UserViewImpl::UserViewImpl(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void UserViewImpl::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Создаем верхнюю панель с кнопкой назад и заголовком
    QHBoxLayout* topBarLayout = new QHBoxLayout();
    backButton = new QPushButton("< Назад");
    QLabel* titleLabel = new QLabel("Профиль пользователя");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    logoutButton = new QPushButton("Выйти");
    logoutButton->setStyleSheet("color: #d9534f;"); // Красный цвет для кнопки выхода

    topBarLayout->addWidget(backButton);
    topBarLayout->addWidget(titleLabel, 1);
      topBarLayout->addWidget(logoutButton);

    // Создаем секцию с информацией о пользователе
    QFrame* profileInfoFrame = new QFrame();
    profileInfoFrame->setFrameShape(QFrame::StyledPanel);
    QGridLayout* profileInfoLayout = new QGridLayout(profileInfoFrame);

    QLabel* usernameTitleLabel = new QLabel("Имя пользователя:");
    usernameLabel = new QLabel("—");
    QLabel* emailTitleLabel = new QLabel("Email:");
    emailLabel = new QLabel("—");
    QLabel* registrationDateTitleLabel = new QLabel("Дата регистрации:");
    registrationDateLabel = new QLabel("—");
    QLabel* favoritesCountTitleLabel = new QLabel("Избранные рецепты:");
    favoritesCountLabel = new QLabel("0");
    QLabel* reviewsCountTitleLabel = new QLabel("Отзывы:");
    reviewsCountLabel = new QLabel("0");

    profileInfoLayout->addWidget(usernameTitleLabel, 0, 0);
    profileInfoLayout->addWidget(usernameLabel, 0, 1);
    profileInfoLayout->addWidget(emailTitleLabel, 1, 0);
    profileInfoLayout->addWidget(emailLabel, 1, 1);
    profileInfoLayout->addWidget(registrationDateTitleLabel, 2, 0);
    profileInfoLayout->addWidget(registrationDateLabel, 2, 1);
    profileInfoLayout->addWidget(favoritesCountTitleLabel, 3, 0);
    profileInfoLayout->addWidget(favoritesCountLabel, 3, 1);
    profileInfoLayout->addWidget(reviewsCountTitleLabel, 4, 0);
    profileInfoLayout->addWidget(reviewsCountLabel, 4, 1);

    // Создаем форму для изменения профиля
    QFrame* updateProfileFrame = new QFrame();
    updateProfileFrame->setFrameShape(QFrame::StyledPanel);
    QGridLayout* updateProfileLayout = new QGridLayout(updateProfileFrame);

    QLabel* newUsernameTitleLabel = new QLabel("Новое имя пользователя:");
    newUsernameLineEdit = new QLineEdit();
    QLabel* newEmailTitleLabel = new QLabel("Новый Email:");
    newEmailLineEdit = new QLineEdit();
    QLabel* currentPasswordTitleLabel = new QLabel("Текущий пароль:");
    currentPasswordLineEdit = new QLineEdit();
    currentPasswordLineEdit->setEchoMode(QLineEdit::Password);
    QLabel* newPasswordTitleLabel = new QLabel("Новый пароль (оставьте пустым, если не нужно менять):");
    newPasswordLineEdit = new QLineEdit();
    newPasswordLineEdit->setEchoMode(QLineEdit::Password);

    updateProfileButton = new QPushButton("Обновить профиль");

    updateProfileLayout->addWidget(newUsernameTitleLabel, 0, 0);
    updateProfileLayout->addWidget(newUsernameLineEdit, 0, 1);
    updateProfileLayout->addWidget(newEmailTitleLabel, 1, 0);
    updateProfileLayout->addWidget(newEmailLineEdit, 1, 1);
    updateProfileLayout->addWidget(currentPasswordTitleLabel, 2, 0);
    updateProfileLayout->addWidget(currentPasswordLineEdit, 2, 1);
    updateProfileLayout->addWidget(newPasswordTitleLabel, 3, 0);
    updateProfileLayout->addWidget(newPasswordLineEdit, 3, 1);
    updateProfileLayout->addWidget(updateProfileButton, 4, 1, Qt::AlignRight);

    // Создаем секцию избранных рецептов
    QLabel* favoritesTitle = new QLabel("Избранные рецепты");
    QFont favoritesTitleFont = favoritesTitle->font();
    favoritesTitleFont.setPointSize(14);
    favoritesTitleFont.setBold(true);
    favoritesTitle->setFont(favoritesTitleFont);

    favoritesScrollArea = new QScrollArea();
    favoritesScrollArea->setWidgetResizable(true);
    favoritesContainer = new QWidget();
    favoritesLayout = new QVBoxLayout(favoritesContainer);
    favoritesScrollArea->setWidget(favoritesContainer);

    // Сборка всего интерфейса
    mainLayout->addLayout(topBarLayout);
    mainLayout->addWidget(profileInfoFrame);
    mainLayout->addWidget(updateProfileFrame);
    mainLayout->addWidget(favoritesTitle);
    mainLayout->addWidget(favoritesScrollArea);

    // Подключение сигналов
    connect(backButton, &QPushButton::clicked, this, &UserViewImpl::backToMainRequested);
    connect(updateProfileButton, &QPushButton::clicked, this, &UserViewImpl::profileUpdateRequested);
    connect(logoutButton, &QPushButton::clicked, this, &UserViewImpl::requestLogout);
}

void UserViewImpl::displayUserProfile(const UserProfileDTO& profile) {
    usernameLabel->setText(profile.username);
    emailLabel->setText(profile.email);
    registrationDateLabel->setText(profile.registrationDate.toString("dd.MM.yyyy"));
    favoritesCountLabel->setText(QString::number(profile.favoriteRecipesCount));
    reviewsCountLabel->setText(QString::number(profile.reviewsCount));

    // Также заполняем поля формы обновления
    newUsernameLineEdit->setText(profile.username);
    newEmailLineEdit->setText(profile.email);
}

void UserViewImpl::displayFavoriteRecipes(const QList<RecipePreviewDTO>& favorites) {
    clearFavoritesList();

    if (favorites.isEmpty()) {
        QLabel* noFavoritesLabel = new QLabel("У вас пока нет избранных рецептов");
        noFavoritesLabel->setAlignment(Qt::AlignCenter);
        favoritesLayout->addWidget(noFavoritesLabel);
    } else {
        for (const auto& recipe : favorites) {
            favoritesLayout->addWidget(createFavoriteRecipeCard(recipe));
        }
    }

    // Добавляем растягивающийся спейсер в конец, чтобы карточки группировались вверху
    favoritesLayout->addStretch();
}

void UserViewImpl::showProfileUpdateSuccess() {
    QMessageBox::information(this, "Обновление профиля", "Профиль успешно обновлен");
    currentPasswordLineEdit->clear();
    newPasswordLineEdit->clear();
}

void UserViewImpl::showProfileUpdateError(const QString& errorMessage) {
    QMessageBox::warning(this, "Ошибка обновления профиля", errorMessage);
}

QString UserViewImpl::getNewUsername() {
    return newUsernameLineEdit->text();
}

QString UserViewImpl::getNewEmail() {
    return newEmailLineEdit->text();
}

QString UserViewImpl::getCurrentPassword() {
    return currentPasswordLineEdit->text();
}

QString UserViewImpl::getNewPassword() {
    return newPasswordLineEdit->text();
}

void UserViewImpl::switchToMainView() {
    emit backToMainRequested();
}

void UserViewImpl::showRecipeDetails(int recipeId) {
    emit recipeSelected(recipeId);
}

void UserViewImpl::clearFavoritesList() {
    // Удаляем все виджеты из контейнера избранных рецептов
    QLayoutItem* child;
    while ((child = favoritesLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
}

QWidget* UserViewImpl::createFavoriteRecipeCard(const RecipePreviewDTO& recipe) {
    // Создаем карточку рецепта
    ClickableFrame* card = new ClickableFrame(recipe.id);
    card->setFrameShape(QFrame::StyledPanel);

    QHBoxLayout* layout = new QHBoxLayout(card);

    // Название рецепта
    QLabel* nameLabel = new QLabel(recipe.name);
    QFont nameFont = nameLabel->font();
    nameFont.setBold(true);
    nameLabel->setFont(nameFont);

    // Рейтинг
    QString ratingText = QString("★ %1").arg(recipe.averageRating, 0, 'f', 1);
    QLabel* ratingLabel = new QLabel(ratingText);
    ratingLabel->setStyleSheet("color: gold;");

    // Кнопка удаления
    QPushButton* removeButton = new QPushButton("✕");
    removeButton->setFixedSize(30, 30);

    layout->addWidget(nameLabel, 1);
    layout->addWidget(ratingLabel);
    layout->addWidget(removeButton);

    // Подключаем обработчик нажатия на карточку
    connect(card, &ClickableFrame::clicked, this, &UserViewImpl::recipeSelected);

    // Подключаем обработчик нажатия на кнопку удаления
    connect(removeButton, &QPushButton::clicked, [this, recipeId = recipe.id]() {
        emit removeFromFavoritesRequested(recipeId);
    });

    return card;
}

void UserViewImpl::requestLogout() {
    // Диалог подтверждения
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Выход из системы",
        "Вы действительно хотите выйти из системы?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        emit logoutRequested();
    }
}

#include "userView.moc"
