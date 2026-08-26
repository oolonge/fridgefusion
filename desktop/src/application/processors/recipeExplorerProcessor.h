#ifndef RECIPEEXPLORERPROCESSOR_H
#define RECIPEEXPLORERPROCESSOR_H

#include "application/interfaces/processors/IRecipeExplorerProcessor.h"
#include "application/interfaces/services/IRecipeFilterService.h"
#include "infrastructure/repositories/IRecipeRepository.h"
#include "infrastructure/repositories/IIngredientRepository.h"
#include <memory>
#include <functional>

class RecipeExplorerProcessor : public IRecipeExplorerProcessor {
public:
    RecipeExplorerProcessor(
        std::shared_ptr<IRecipeRepository> recipeRepository,
        std::shared_ptr<IIngredientRepository> ingredientRepository,
        std::shared_ptr<IRecipeFilterService> recipeFilterService);

    ~RecipeExplorerProcessor() override = default;

    // Новые методы
    QList<RecipePreviewDTO> getAllRecipePreviews(int limit = 100, int offset = 0) override;
    QList<Category> getAllCategories() override;
    QList<Ingredient> getAllIngredients() override;

    // Существующие методы
    QList<Recipe> findRecipesWithIngredients(const QList<int>& ingredientIds) override;
    QList<Recipe> findRecipesByCategory(int categoryId) override;
    QList<Recipe> findRecipesByCategoryAndIngredients(int categoryId, const QList<int>& ingredientIds) override;
    QList<std::pair<int, double>> getIngredientSubstitutes(int ingredientId, int limit = 3) override;
    RecipePreviewDTO getRecipePreview(int recipeId) override;
    RecipeDetailDTO getRecipeDetail(int recipeId) override;

private:
    std::shared_ptr<IRecipeRepository> recipeRepository;
    std::shared_ptr<IIngredientRepository> ingredientRepository;
    std::shared_ptr<IRecipeFilterService> recipeFilterService;

    // Вспомогательные методы для поиска с заменами
    QList<QList<int>> generateIngredientCombinations(const QList<int>& ingredientIds);
    QList<Recipe> findRecipesWithIngredientCombinations(const QList<QList<int>>& ingredientCombinations);
    double calculateSimilarityScore(const QList<int>& originalIds, const QList<int>& substituteIds);
};

#endif // RECIPEEXPLORERPROCESSOR_H
