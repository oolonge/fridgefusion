#ifndef RECIPEDETAILPRESENTER_H
#define RECIPEDETAILPRESENTER_H

#include "application/interfaces/processors/IRecipeExplorerProcessor.h"
#include "application/interfaces/processors/IFavoriteProcessor.h"
#include "application/interfaces/processors/IReviewProcessor.h"
#include "application/interfaces/processors/IAuthProcessor.h"
#include "presentation/views/recipeDetailView.h"
#include <memory>

class RecipeDetailPresenter {
public:
    RecipeDetailPresenter(
        std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor,
        std::shared_ptr<IFavoriteProcessor> favoriteProcessor,
        std::shared_ptr<IReviewProcessor> reviewProcessor,
        std::shared_ptr<IAuthProcessor> authProcessor,
        RecipeDetailView* view);
    ~RecipeDetailPresenter() = default;

    void loadRecipeDetails(int recipeId);
    void loadReviews(int recipeId);
    void toggleFavorite(int recipeId);
    void submitReview(int recipeId);
    void notifyFavoriteToggleResult(bool success, bool newStatus);

private:
    std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor;
    std::shared_ptr<IFavoriteProcessor> favoriteProcessor;
    std::shared_ptr<IReviewProcessor> reviewProcessor;
    std::shared_ptr<IAuthProcessor> authProcessor;
    RecipeDetailView* view;
};

#endif // RECIPEDETAILPRESENTER_H
