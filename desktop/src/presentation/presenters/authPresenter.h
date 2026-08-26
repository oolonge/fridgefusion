#ifndef AUTHPRESENTER_H
#define AUTHPRESENTER_H

#include "application/interfaces/processors/IAuthProcessor.h"
#include "presentation/views/authView.h"
#include <memory>

class AuthPresenter {
public:
    AuthPresenter(std::shared_ptr<IAuthProcessor> authProcessor, AuthView* view);
    ~AuthPresenter() = default;

    void login();
    void registerUser();
    void logout();
    bool isUserLoggedIn() const;
    int getCurrentUserId() const;

private:
    std::shared_ptr<IAuthProcessor> authProcessor;
    AuthView* view;
};

#endif // AUTHPRESENTER_H
