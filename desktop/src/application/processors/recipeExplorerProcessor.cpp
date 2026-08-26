#include "recipeExplorerProcessor.h"
#include <QDebug>
#include <algorithm>
#include <unordered_set>

RecipeExplorerProcessor::RecipeExplorerProcessor(
    std::shared_ptr<IRecipeRepository> recipeRepository,
    std::shared_ptr<IIngredientRepository> ingredientRepository,
    std::shared_ptr<IRecipeFilterService> recipeFilterService)
    : recipeRepository(std::move(recipeRepository)),
    ingredientRepository(std::move(ingredientRepository)),
    recipeFilterService(std::move(recipeFilterService)) {
}

QList<Recipe> RecipeExplorerProcessor::findRecipesWithIngredients(const QList<int>& ingredientIds) {
    if (ingredientIds.isEmpty()) {
        return QList<Recipe>();
    }

    // Генерируем все возможные комбинации ингредиентов с заменами
    QList<QList<int>> ingredientCombinations = generateIngredientCombinations(ingredientIds);

    // Ищем рецепты для всех комбинаций
    return findRecipesWithIngredientCombinations(ingredientCombinations);
}

QList<RecipePreviewDTO> RecipeExplorerProcessor::getAllRecipePreviews(int limit, int offset) {
    QList<RecipePreviewDTO> result;
    QList<Recipe> recipes = recipeRepository->getAllRecipes(limit, offset);

    // Преобразуем Recipe в RecipePreviewDTO
    for (const Recipe& recipe : recipes) {
        RecipePreviewDTO preview = recipeRepository->getRecipePreview(recipe.getRecipeId());
        result.append(preview);
    }

    return result;
}

QList<Category> RecipeExplorerProcessor::getAllCategories() {
    return recipeRepository->getAllCategories();
}

QList<Ingredient> RecipeExplorerProcessor::getAllIngredients() {
    return ingredientRepository->getAllIngredients();
}

QList<Recipe> RecipeExplorerProcessor::findRecipesByCategory(int categoryId) {
    return recipeRepository->getRecipesByCategory(categoryId);
}

QList<Recipe> RecipeExplorerProcessor::findRecipesByCategoryAndIngredients(int categoryId, const QList<int>& ingredientIds) {
    // Сначала ищем рецепты по ингредиентам
    QList<Recipe> recipesByIngredients = findRecipesWithIngredients(ingredientIds);

    // Затем фильтруем их по категории
    RecipeFilterDTO filterDTO;
    QList<int> categoryIds;
    categoryIds.append(categoryId);
    filterDTO.setCategoryIds(categoryIds);

    return recipeFilterService->filterRecipes(recipesByIngredients, filterDTO);
}

QList<std::pair<int, double>> RecipeExplorerProcessor::getIngredientSubstitutes(int ingredientId, int limit) {
    return ingredientRepository->getIngredientSubstitutes(ingredientId, 0.5, limit);
}

RecipePreviewDTO RecipeExplorerProcessor::getRecipePreview(int recipeId) {
    return recipeRepository->getRecipePreview(recipeId);
}

RecipeDetailDTO RecipeExplorerProcessor::getRecipeDetail(int recipeId) {
    return recipeRepository->getRecipeDetail(recipeId);
}

QList<QList<int>> RecipeExplorerProcessor::generateIngredientCombinations(const QList<int>& ingredientIds) {
    QList<QList<int>> result;
    QList<QList<std::pair<int, double>>> allSubstitutes;

    // Для каждого ингредиента получаем список возможных заменителей
    for (int ingredientId : ingredientIds) {
        QList<std::pair<int, double>> substitutes = ingredientRepository->getIngredientSubstitutes(ingredientId);

        // Добавляем сам ингредиент как лучшую замену (score = 1.0)
        substitutes.prepend(std::make_pair(ingredientId, 1.0));

        allSubstitutes.append(substitutes);
    }

    // Генерируем все возможные комбинации (не более 8)
    int maxCombinations = 8;
    int totalCombinations = 1;
    for (const auto& substList : allSubstitutes) {
        totalCombinations *= substList.size();
    }
    bool limitCombinations = (totalCombinations > maxCombinations);

    // Добавляем исходную комбинацию без замен
    QList<int> originalCombo;
    for (int id : ingredientIds) {
        originalCombo.append(id);
    }
    result.append(originalCombo);

    // Если у нас всего один ингредиент или общее число комбинаций небольшое,
    // мы можем просто сгенерировать все возможные комбинации
    if (ingredientIds.size() == 1 || (!limitCombinations && totalCombinations <= maxCombinations)) {
        // Рекурсивно генерируем все комбинации
        QList<int> currentCombo;
        QList<QList<int>> combinations;

        std::function<void(int, QList<int>&)> generateCombos = [&](int depth, QList<int>& combo) {
            if (depth == allSubstitutes.size()) {
                combinations.append(combo);
                return;
            }

            for (const auto& substitute : allSubstitutes[depth]) {
                combo.append(substitute.first);
                generateCombos(depth + 1, combo);
                combo.removeLast();
            }
        };

        generateCombos(0, currentCombo);

        // Убираем исходную комбинацию, так как мы уже добавили её
        for (int i = 1; i < combinations.size(); ++i) {
            result.append(combinations[i]);
        }
    } else {
        // Для большого числа ингредиентов или комбинаций используем более эффективный подход:
        // Включаем только комбинации с одной заменой
        for (int i = 0; i < ingredientIds.size(); ++i) {
            // Для каждого ингредиента пробуем все его заменители
            for (int j = 1; j < allSubstitutes[i].size() && result.size() < maxCombinations; ++j) {
                QList<int> combo = originalCombo;
                combo[i] = allSubstitutes[i][j].first;
                result.append(combo);
            }
        }
    }

    // Сортируем комбинации по убыванию суммарного score похожести
    std::sort(result.begin(), result.end(), [this, &ingredientIds](const QList<int>& a, const QList<int>& b) {
        double scoreA = this->calculateSimilarityScore(ingredientIds, a);
        double scoreB = this->calculateSimilarityScore(ingredientIds, b);
        return scoreA > scoreB;
    });

    return result;
}

QList<Recipe> RecipeExplorerProcessor::findRecipesWithIngredientCombinations(const QList<QList<int>>& ingredientCombinations) {
    // Используем множество для избежания дубликатов рецептов
    std::unordered_set<int> uniqueRecipeIds;
    QList<Recipe> allRecipes;

    // Ищем рецепты для каждой комбинации ингредиентов
    for (const QList<int>& combo : ingredientCombinations) {
        QList<Recipe> recipes = recipeRepository->getRecipesByIngredients(combo);

        for (const Recipe& recipe : recipes) {
            if (uniqueRecipeIds.find(recipe.getRecipeId()) == uniqueRecipeIds.end()) {
                uniqueRecipeIds.insert(recipe.getRecipeId());
                allRecipes.append(recipe);
            }
        }
    }

    return allRecipes;
}

double RecipeExplorerProcessor::calculateSimilarityScore(const QList<int>& originalIds, const QList<int>& substituteIds) {
    if (originalIds.size() != substituteIds.size()) {
        return 0.0;
    }

    double totalScore = 0.0;

    for (int i = 0; i < originalIds.size(); ++i) {
        if (originalIds[i] == substituteIds[i]) {
            totalScore += 1.0;
        } else {
            // Ищем коэффициент похожести для замены
            QList<std::pair<int, double>> substitutes = ingredientRepository->getIngredientSubstitutes(originalIds[i]);
            bool found = false;

            for (const auto& substitute : substitutes) {
                if (substitute.first == substituteIds[i]) {
                    totalScore += substitute.second;
                    found = true;
                    break;
                }
            }

            if (!found) {
                totalScore += 0.0; // Если замена не найдена в списке известных заменителей
            }
        }
    }

    return totalScore / originalIds.size();
}
