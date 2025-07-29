#ifndef IADMINPROCESSOR_H
#define IADMINPROCESSOR_H

#include <QList>
#include "domain/entities/user.h"
#include "domain/dtos/userProfileDTO.h"
#include "domain/entities/recipe.h"
#include "domain/dtos/recipeDetailDTO.h"

class IAdminProcessor {
public:
    virtual ~IAdminProcessor() = default;

    // Методы для управления пользователями
    virtual QList<User> getAllUsers(int limit = 100, int offset = 0) = 0;
    virtual QList<User> searchUsers(const QString& searchTerm) = 0;
    virtual QList<User> getUsersByRole(UserRole role) = 0;
    virtual std::optional<UserProfileDTO> getUserProfile(int userId) = 0;
    virtual bool updateUserRole(int userId, UserRole role) = 0;
    virtual bool deleteUser(int userId) = 0;
    virtual int getTotalUsersCount() = 0;
    virtual int getUsersCountByRole(UserRole role) = 0;

    // Методы для управления рецептами
    virtual bool updateRecipe(int recipeId, const QString& name, const QString& description, 
                             int preparationTime, int cookingTime, const QString& instructions) = 0;


    virtual bool deleteRecipe(int recipeId) = 0;
    virtual int createRecipe(const QString& name, const QString& description = "",
                             int preparationTime = 10, int cookingTime = 20,
                             const QString& instructions = "", int authorId = -1) = 0;

    virtual bool addCategoryToRecipe(int recipeId, int categoryId) = 0;
    virtual bool removeCategoryFromRecipe(int recipeId, int categoryId) = 0;
    virtual bool addIngredientToRecipe(int recipeId, int ingredientId, double quantity, 
                                      const QString& unit, bool isOptional = false) = 0;
    virtual bool removeIngredientFromRecipe(int recipeId, int ingredientId) = 0;
    virtual bool updateIngredientQuantity(int recipeId, int ingredientId, double quantity) = 0;



};

#endif // IADMINPROCESSOR_H
