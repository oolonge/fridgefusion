#ifndef AUTHVIEW_H
#define AUTHVIEW_H

#include <QString>

class AuthView {
public:
    virtual ~AuthView() = default;

    // Методы для обновления представления
    virtual void showLoginError(const QString& errorMessage) = 0;
    virtual void showRegistrationError(const QString& errorMessage) = 0;
    virtual void clearLoginForm() = 0;
    virtual void clearRegistrationForm() = 0;
    virtual void switchToMainView() = 0;

    // Методы для получения данных из формы
    virtual QString getLoginEmail() = 0;
    virtual QString getLoginPassword() = 0;
    virtual QString getRegistrationUsername() = 0;
    virtual QString getRegistrationEmail() = 0;
    virtual QString getRegistrationPassword() = 0;
    virtual QString getRegistrationConfirmPassword() = 0;

    // Обновления состояния UI
    virtual void updateLoginState(bool isLoggedIn, const QString& username) = 0;
};

#endif // AUTHVIEW_H
