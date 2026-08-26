#include "authView.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QMessageBox>

class AuthViewImpl : public QWidget, public AuthView {
    Q_OBJECT

public:
    explicit AuthViewImpl(QWidget* parent = nullptr);
    ~AuthViewImpl() override = default;

    // Реализация интерфейса AuthView
    void showLoginError(const QString& errorMessage) override;
    void showRegistrationError(const QString& errorMessage) override;
    void clearLoginForm() override;
    void clearRegistrationForm() override;
    void switchToMainView() override;
    QString getLoginEmail() override;
    QString getLoginPassword() override;
    QString getRegistrationUsername() override;
    QString getRegistrationEmail() override;
    QString getRegistrationPassword() override;
    QString getRegistrationConfirmPassword() override;
    void updateLoginState(bool isLoggedIn, const QString& username) override;

signals:
    void loginRequested();
    void registerRequested();
    void backToMainRequested();
    void loginStateChanged(bool isLoggedIn, const QString& username);


private:
    // UI элементы
    QTabWidget* authTabWidget;
    QWidget* loginTab;
    QWidget* registerTab;
    QLineEdit* loginEmailLineEdit;
    QLineEdit* loginPasswordLineEdit;
    QLineEdit* registerUsernameLineEdit;
    QLineEdit* registerEmailLineEdit;
    QLineEdit* registerPasswordLineEdit;
    QLineEdit* registerConfirmPasswordLineEdit;
    QPushButton* loginSubmitButton;
    QPushButton* registerSubmitButton;
    QPushButton* forgotPasswordButton;
    QPushButton* backToMainButton;

    void setupUi();
};

AuthViewImpl::AuthViewImpl(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

void AuthViewImpl::setupUi() {
    // Создание основного layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Создание заголовка
    QLabel* titleLabel = new QLabel("FridgeFusion");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    // Создание TabWidget для вкладок входа и регистрации
    authTabWidget = new QTabWidget(this);
    loginTab = new QWidget();
    registerTab = new QWidget();

    // Настройка вкладки входа
    QVBoxLayout* loginLayout = new QVBoxLayout(loginTab);
    QLabel* loginEmailLabel = new QLabel("Email:");
    loginEmailLineEdit = new QLineEdit();
    loginEmailLineEdit->setPlaceholderText("Введите ваш email");

    QLabel* loginPasswordLabel = new QLabel("Пароль:");
    loginPasswordLineEdit = new QLineEdit();
    loginPasswordLineEdit->setPlaceholderText("Введите ваш пароль");
    loginPasswordLineEdit->setEchoMode(QLineEdit::Password);

    loginSubmitButton = new QPushButton("Войти");
    forgotPasswordButton = new QPushButton("Забыли пароль?");
    forgotPasswordButton->setFlat(true);

    loginLayout->addWidget(loginEmailLabel);
    loginLayout->addWidget(loginEmailLineEdit);
    loginLayout->addWidget(loginPasswordLabel);
    loginLayout->addWidget(loginPasswordLineEdit);
    loginLayout->addWidget(loginSubmitButton);
    loginLayout->addWidget(forgotPasswordButton);
    loginLayout->addStretch();

    // Настройка вкладки регистрации
    QVBoxLayout* registerLayout = new QVBoxLayout(registerTab);
    QLabel* registerUsernameLabel = new QLabel("Имя пользователя:");
    registerUsernameLineEdit = new QLineEdit();
    registerUsernameLineEdit->setPlaceholderText("Введите имя пользователя");

    QLabel* registerEmailLabel = new QLabel("Email:");
    registerEmailLineEdit = new QLineEdit();
    registerEmailLineEdit->setPlaceholderText("Введите ваш email");

    QLabel* registerPasswordLabel = new QLabel("Пароль:");
    registerPasswordLineEdit = new QLineEdit();
    registerPasswordLineEdit->setPlaceholderText("Введите пароль");
    registerPasswordLineEdit->setEchoMode(QLineEdit::Password);

    QLabel* registerConfirmPasswordLabel = new QLabel("Подтверждение пароля:");
    registerConfirmPasswordLineEdit = new QLineEdit();
    registerConfirmPasswordLineEdit->setPlaceholderText("Повторите пароль");
    registerConfirmPasswordLineEdit->setEchoMode(QLineEdit::Password);

    registerSubmitButton = new QPushButton("Зарегистрироваться");

    registerLayout->addWidget(registerUsernameLabel);
    registerLayout->addWidget(registerUsernameLineEdit);
    registerLayout->addWidget(registerEmailLabel);
    registerLayout->addWidget(registerEmailLineEdit);
    registerLayout->addWidget(registerPasswordLabel);
    registerLayout->addWidget(registerPasswordLineEdit);
    registerLayout->addWidget(registerConfirmPasswordLabel);
    registerLayout->addWidget(registerConfirmPasswordLineEdit);
    registerLayout->addWidget(registerSubmitButton);
    registerLayout->addStretch();

    // Добавление вкладок в TabWidget
    authTabWidget->addTab(loginTab, "Вход");
    authTabWidget->addTab(registerTab, "Регистрация");

    // Кнопка для возврата на главный экран (для отладки)
    backToMainButton = new QPushButton("Назад (без входа)");

    // Сборка всего интерфейса
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(authTabWidget);
    mainLayout->addWidget(backToMainButton);

    // Подключение сигналов
    connect(loginSubmitButton, &QPushButton::clicked, this, &AuthViewImpl::loginRequested);
    connect(registerSubmitButton, &QPushButton::clicked, this, &AuthViewImpl::registerRequested);
    connect(backToMainButton, &QPushButton::clicked, this, &AuthViewImpl::backToMainRequested);
}

void AuthViewImpl::showLoginError(const QString& errorMessage) {
    QMessageBox::warning(this, "Ошибка входа", errorMessage);
}

void AuthViewImpl::showRegistrationError(const QString& errorMessage) {
    QMessageBox::warning(this, "Ошибка регистрации", errorMessage);
}

void AuthViewImpl::clearLoginForm() {
    loginEmailLineEdit->clear();
    loginPasswordLineEdit->clear();
}

void AuthViewImpl::clearRegistrationForm() {
    registerUsernameLineEdit->clear();
    registerEmailLineEdit->clear();
    registerPasswordLineEdit->clear();
    registerConfirmPasswordLineEdit->clear();
}

void AuthViewImpl::switchToMainView() {
    emit backToMainRequested();
}

QString AuthViewImpl::getLoginEmail() {
    return loginEmailLineEdit->text();
}

QString AuthViewImpl::getLoginPassword() {
    return loginPasswordLineEdit->text();
}

QString AuthViewImpl::getRegistrationUsername() {
    return registerUsernameLineEdit->text();
}

QString AuthViewImpl::getRegistrationEmail() {
    return registerEmailLineEdit->text();
}

QString AuthViewImpl::getRegistrationPassword() {
    return registerPasswordLineEdit->text();
}

QString AuthViewImpl::getRegistrationConfirmPassword() {
    return registerConfirmPasswordLineEdit->text();
}

void AuthViewImpl::updateLoginState(bool isLoggedIn, const QString& username) {
    emit loginStateChanged(isLoggedIn, username);
}

#include "authView.moc"
