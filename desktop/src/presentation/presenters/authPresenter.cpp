#include "authPresenter.h"

AuthPresenter::AuthPresenter(std::shared_ptr<IAuthProcessor> authProcessor, AuthView* view)
    : authProcessor(std::move(authProcessor)), view(view) {
}

void AuthPresenter::login() {
    QString email = view->getLoginEmail();
    QString password = view->getLoginPassword();

    if (email.isEmpty() || password.isEmpty()) {
        view->showLoginError("Email и пароль не могут быть пустыми");
        return;
    }

    AuthResult result = authProcessor->login(email, password);

    if (result.isSuccess()) {
        view->clearLoginForm();
        view->updateLoginState(true, result.getUser()->getUsername());
        view->switchToMainView();
    } else {
        view->showLoginError(result.getErrorMessage());
    }
}

void AuthPresenter::registerUser() {
    QString username = view->getRegistrationUsername();
    QString email = view->getRegistrationEmail();
    QString password = view->getRegistrationPassword();
    QString confirmPassword = view->getRegistrationConfirmPassword();

    if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
        view->showRegistrationError("Все поля должны быть заполнены");
        return;
    }

    if (password != confirmPassword) {
        view->showRegistrationError("Пароли не совпадают");
        return;
    }

    AuthResult result = authProcessor->registerUser(username, email, password);

    if (result.isSuccess()) {
        view->clearRegistrationForm();
        view->updateLoginState(true, result.getUser()->getUsername());
        view->switchToMainView();
    } else {
        view->showRegistrationError(result.getErrorMessage());
    }
}

void AuthPresenter::logout() {
    if (authProcessor->logout()) {
        view->updateLoginState(false, "");
    }
}

bool AuthPresenter::isUserLoggedIn() const {
    return authProcessor->isUserLoggedIn();
}

int AuthPresenter::getCurrentUserId() const {
    return authProcessor->getCurrentUserId();
}
