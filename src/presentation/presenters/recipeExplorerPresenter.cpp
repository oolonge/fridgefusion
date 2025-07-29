#include "recipeExplorerPresenter.h"
#include "domain/dtos/recipeFilterDTO.h"

RecipeExplorerPresenter::RecipeExplorerPresenter(
    std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor,
    std::shared_ptr<IFavoriteProcessor> favoriteProcessor,
    std::shared_ptr<IReviewProcessor> reviewProcessor,
    RecipeExplorerView* view)
    : recipeProcessor(std::move(recipeProcessor)),
    favoriteProcessor(std::move(favoriteProcessor)),
    reviewProcessor(std::move(reviewProcessor)),
    view(view) {
}

void RecipeExplorerPresenter::searchRecipes() {
    QList<int> ingredientIds = view->getSelectedIngredientIds();
    QList<int> categoryIds = view->getSelectedCategoryIds();
    int maxCookingTime = view->getMaxCookingTime();

    QList<Recipe> recipes;

    if (!ingredientIds.isEmpty() && !categoryIds.isEmpty()) {
        recipes = recipeProcessor->findRecipesByCategoryAndIngredients(categoryIds.first(), ingredientIds);
    } else if (!ingredientIds.isEmpty()) {
        recipes = recipeProcessor->findRecipesWithIngredients(ingredientIds);
    } else if (!categoryIds.isEmpty()) {
        recipes = recipeProcessor->findRecipesByCategory(categoryIds.first());
    } else {
        // По умолчанию загружаем все рецепты (если возможно)
        loadAllRecipes();
        return;
    }

    // Преобразование Recipe в RecipePreviewDTO
    QList<RecipePreviewDTO> recipePreviews;
    for (const auto& recipe : recipes) {
        RecipePreviewDTO preview = recipeProcessor->getRecipePreview(recipe.getRecipeId());
        recipePreviews.append(preview);
    }

    if (recipePreviews.isEmpty()) {
        view->showNoRecipesFound();
    } else {
        view->displaySearchResults(recipePreviews);
    }
}

void RecipeExplorerPresenter::loadRecipeDetails(int recipeId) {
    // Здесь должна быть загрузка деталей рецепта
    // и переход к его отображению
    view->showRecipeDetails(recipeId);
}

void RecipeExplorerPresenter::addToFavorites(int userId, int recipeId) {
    favoriteProcessor->addToFavorites(userId, recipeId);
}

void RecipeExplorerPresenter::removeFromFavorites(int userId, int recipeId) {
    favoriteProcessor->removeFromFavorites(userId, recipeId);
}

void RecipeExplorerPresenter::loadAllRecipes() {
    try {
        // Получаем все превью рецептов из бизнес-логики
        QList<RecipePreviewDTO> recipes = recipeProcessor->getAllRecipePreviews(100, 0);

        if (recipes.isEmpty()) {
            view->showNoRecipesFound();
        } else {
            view->displayRecipes(recipes);
        }
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при загрузке рецептов:" << e.what();
        view->showNoRecipesFound();
    }
}

void RecipeExplorerPresenter::loadCategories() {
    try {
        // Получаем все категории из бизнес-логики
        QList<Category> categories = recipeProcessor->getAllCategories();
        view->displayCategories(categories);
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при загрузке категорий:" << e.what();
        // Временная заглушка для отображения
        QList<Category> dummyCategories;
        view->displayCategories(dummyCategories);
    }
}

void RecipeExplorerPresenter::loadIngredients() {
    try {
        // Получаем все ингредиенты из бизнес-логики
        QList<Ingredient> ingredients = recipeProcessor->getAllIngredients();
        view->displayIngredients(ingredients);
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при загрузке ингредиентов:" << e.what();
        // Временная заглушка для отображения
        QList<Ingredient> dummyIngredients;
        view->displayIngredients(dummyIngredients);
    }
}
