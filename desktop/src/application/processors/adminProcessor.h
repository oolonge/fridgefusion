#ifndef ADMINPROCESSOR_H
#define ADMINPROCESSOR_H

#include "application/interfaces/processors/IAdminProcessor.h"
#include "infrastructure/repositories/IUserRepository.h"
#include "infrastructure/repositories/IFavoriteRepository.h"
#include "infrastructure/repositories/IReviewRepository.h"
#include "infrastructure/repositories/IRecipeRepository.h"
#include "utils/logging/ILogger.h"
#include <memory>

class AdminProcessor : public IAdminProcessor {
public:
    explicit AdminProcessor(
        std::shared_ptr<IUserRepository> userRepository,
        std::shared_ptr<IFavoriteRepository> favoriteRepository,
        std::shared_ptr<IReviewRepository> reviewRepository,
        std::shared_ptr<IRecipeRepository> recipeRepository,
        std::shared_ptr<ILogger> logger = nullptr);

    ~AdminProcessor() override = default;

    // Управление пользователями
    QList<User> getAllUsers(int limit = 100, int offset = 0) override;
    QList<User> searchUsers(const QString& searchTerm) override;
    QList<User> getUsersByRole(UserRole role) override;
    std::optional<UserProfileDTO> getUserProfile(int userId) override;
    bool updateUserRole(int userId, UserRole role) override;
    bool deleteUser(int userId) override;
    int getTotalUsersCount() override;
    int getUsersCountByRole(UserRole role) override;

    // Управление рецептами
    bool updateRecipe(int recipeId, const QString& name, const QString& description, 
                     int preparationTime, int cookingTime, const QString& instructions) override;

    bool deleteRecipe(int recipeId) override;
    int createRecipe(const QString& name, const QString& description = "",
                     int preparationTime = 10, int cookingTime = 20,
                     const QString& instructions = "", int authorId = -1) override;

    bool addCategoryToRecipe(int recipeId, int categoryId) override;
    bool removeCategoryFromRecipe(int recipeId, int categoryId) override;
    bool addIngredientToRecipe(int recipeId, int ingredientId, double quantity, 
                              const QString& unit, bool isOptional = false) override;
    bool removeIngredientFromRecipe(int recipeId, int ingredientId) override;
    bool updateIngredientQuantity(int recipeId, int ingredientId, double quantity) override;

private:
    std::shared_ptr<IUserRepository> userRepository;
    std::shared_ptr<IFavoriteRepository> favoriteRepository;
    std::shared_ptr<IReviewRepository> reviewRepository;
    std::shared_ptr<IRecipeRepository> recipeRepository;
    std::shared_ptr<ILogger> logger;
};

#endif // ADMINPROCESSOR_H
