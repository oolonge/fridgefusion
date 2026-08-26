#ifndef USERVIEW_H
#define USERVIEW_H

#include <QString>
#include "domain/dtos/userProfileDTO.h"
#include "domain/dtos/recipePreviewDTO.h"

class UserView {
public:
    virtual ~UserView() = default;

    // Методы для обновления представления
    virtual void displayUserProfile(const UserProfileDTO& profile) = 0;
    virtual void displayFavoriteRecipes(const QList<RecipePreviewDTO>& favorites) = 0;
    virtual void showProfileUpdateSuccess() = 0;
    virtual void showProfileUpdateError(const QString& errorMessage) = 0;
    virtual void showRecipeDetails(int recipeId) = 0;
    virtual void requestLogout() = 0;

    // Методы для получения данных из формы
    virtual QString getNewUsername() = 0;
    virtual QString getNewEmail() = 0;
    virtual QString getCurrentPassword() = 0;
    virtual QString getNewPassword() = 0;

    // Навигация
    virtual void switchToMainView() = 0;
};

#endif // USERVIEW_H
