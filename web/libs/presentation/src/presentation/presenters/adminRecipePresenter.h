#ifndef ADMINRECIPEPRESENTER_H
#define ADMINRECIPEPRESENTER_H

#include "application/interfaces/processors/IRecipeExplorerProcessor.h"
#include "application/interfaces/processors/IAuthProcessor.h"
#include "application/interfaces/processors/IAdminProcessor.h"
#include "presentation/views/adminRecipeView.h"
#include <memory>

class AdminRecipePresenter {
public:
    AdminRecipePresenter(
        std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor,
        std::shared_ptr<IAuthProcessor> authProcessor,
        std::shared_ptr<IAdminProcessor> adminProcessor,
        AdminRecipeView* view);
    ~AdminRecipePresenter() = default;

    void loadAllRecipes(int limit = 100, int offset = 0);
    void searchRecipes(const QString& searchTerm);
    void loadRecipeDetails(int recipeId);
    void updateRecipe(int recipeId);
    void deleteRecipe(int recipeId);
    void loadCategories();
    void loadIngredients();
    void addCategoryToRecipe(int recipeId, int categoryId);
    void removeCategoryFromRecipe(int recipeId, int categoryId);
    void addIngredientToRecipe(int recipeId, int ingredientId, double quantity);
    void removeIngredientFromRecipe(int recipeId, int ingredientId);
    void updateIngredientQuantity(int recipeId, int ingredientId, double quantity);
    bool checkAdminAccess();

    void createRecipe(const QString& name);

private:
    std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor;
    std::shared_ptr<IAuthProcessor> authProcessor;
    std::shared_ptr<IAdminProcessor> adminProcessor;
    AdminRecipeView* view;
};

#endif // ADMINRECIPEPRESENTER_H
