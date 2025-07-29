#ifndef USERPRESENTER_H
#define USERPRESENTER_H

#include "application/interfaces/processors/IUserProcessor.h"
#include "application/interfaces/processors/IFavoriteProcessor.h"
#include "application/interfaces/processors/IRecipeExplorerProcessor.h" // Добавляем
#include "presentation/views/userView.h"
#include <memory>

class UserPresenter {
public:
    UserPresenter(
        std::shared_ptr<IUserProcessor> userProcessor,
        std::shared_ptr<IFavoriteProcessor> favoriteProcessor,
        std::shared_ptr<IRecipeExplorerProcessor> recipeExplorerProcessor, // Добавляем
        UserView* view);
    ~UserPresenter() = default;

    void loadUserProfile(int userId);
    void loadFavoriteRecipes(int userId);
    void updateUserProfile(int userId);
    void removeFromFavorites(int userId, int recipeId);

private:
    std::shared_ptr<IUserProcessor> userProcessor;
    std::shared_ptr<IFavoriteProcessor> favoriteProcessor;
    std::shared_ptr<IRecipeExplorerProcessor> recipeExplorerProcessor; // Добавляем
    UserView* view;
};

#endif // USERPRESENTER_H
