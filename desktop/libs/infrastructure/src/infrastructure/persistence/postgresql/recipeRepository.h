#ifndef RECIPEREPOSITORY_H
#define RECIPEREPOSITORY_H

#include "infrastructure/repositories/IRecipeRepository.h"
#include "infrastructure/persistence/database/dbContext.h"
#include <memory>

class RecipeRepository : public IRecipeRepository {
public:
    explicit RecipeRepository(std::shared_ptr<DbContext> dbContext);
    ~RecipeRepository() override = default;

    // Базовые CRUD операции
    std::optional<Recipe> getRecipeById(int recipeId) override;
    QList<Recipe> getAllRecipes(int limit = 100, int offset = 0) override;
    bool updateRecipe(int recipeId, const QString& name, const QString& description, 
                     int preparationTime, int cookingTime, const QString& instructions) override;
    bool deleteRecipe(int recipeId) override;

    // Категории рецептов
    QList<Category> getAllCategories() override;
    bool addCategoryToRecipe(int recipeId, int categoryId) override;
    bool removeCategoryFromRecipe(int recipeId, int categoryId) override;

    // Ингредиенты рецептов
    QList<RecipeIngredient> getRecipeIngredients(int recipeId) override;
    bool addIngredientToRecipe(int recipeId, int ingredientId, double quantity, 
                             const QString& unit, bool isOptional = false) override;
    bool removeIngredientFromRecipe(int recipeId, int ingredientId) override;
    bool updateIngredientQuantity(int recipeId, int ingredientId, double quantity) override;

    // Поиск рецептов
    QList<Recipe> getRecipesByIngredients(const QList<int>& ingredientIds) override;
    QList<Recipe> getRecipesByCategory(int categoryId) override;

    // Поиск заменителей ингредиентов
    QList<std::pair<int, double>> getIngredientSubstitutes(int ingredientId, int limit = 3) override;

    // Получение подробной информации для DTO
    RecipePreviewDTO getRecipePreview(int recipeId) override;
    RecipeDetailDTO getRecipeDetail(int recipeId) override;

    int createRecipe(const QString& name, const QString& description,
                     int preparationTime, int cookingTime,
                     const QString& instructions, int authorId) override;

private:
    std::shared_ptr<DbContext> dbContext;

    // Вспомогательные методы для преобразования записей БД в объекты
    Recipe mapToRecipe(const QSqlRecord& record);
    RecipeIngredient mapToRecipeIngredient(const QSqlRecord& record);
    QList<int> getRecipeCategoryIds(int recipeId);
    int getUnitIdByName(const QString& unitName);
};

#endif // RECIPEREPOSITORY_H
