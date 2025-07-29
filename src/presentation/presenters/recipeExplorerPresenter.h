#ifndef RECIPEEXPLORERPRESENTER_H
#define RECIPEEXPLORERPRESENTER_H

#include "application/interfaces/processors/IRecipeExplorerProcessor.h"
#include "application/interfaces/processors/IFavoriteProcessor.h"
#include "application/interfaces/processors/IReviewProcessor.h"
#include "presentation/views/recipeExplorerView.h"
#include <memory>

class RecipeExplorerPresenter {
public:
    RecipeExplorerPresenter(
        std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor,
        std::shared_ptr<IFavoriteProcessor> favoriteProcessor,
        std::shared_ptr<IReviewProcessor> reviewProcessor,
        RecipeExplorerView* view);
    ~RecipeExplorerPresenter() = default;

    void loadAllRecipes();
    void loadCategories();
    void loadIngredients();
    void searchRecipes();
    void loadRecipeDetails(int recipeId);
    void addToFavorites(int userId, int recipeId);
    void removeFromFavorites(int userId, int recipeId);

private:
    std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor;
    std::shared_ptr<IFavoriteProcessor> favoriteProcessor;
    std::shared_ptr<IReviewProcessor> reviewProcessor;
    RecipeExplorerView* view;
};

#endif // RECIPEEXPLORERPRESENTER_H
