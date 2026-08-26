#include "adminProcessor.h"
#include "utils/logging/LoggerService.h"
#include <QDebug>

AdminProcessor::AdminProcessor(
    std::shared_ptr<IUserRepository> userRepository,
    std::shared_ptr<IFavoriteRepository> favoriteRepository,
    std::shared_ptr<IReviewRepository> reviewRepository,
    std::shared_ptr<IRecipeRepository> recipeRepository,
    std::shared_ptr<ILogger> logger)
    : userRepository(std::move(userRepository)),
    favoriteRepository(std::move(favoriteRepository)),
    reviewRepository(std::move(reviewRepository)),
    recipeRepository(std::move(recipeRepository)),
    logger(logger ? logger : LoggerService::getInstance().getLogger("default")) {
}

QList<User> AdminProcessor::getAllUsers(int limit, int offset) {
    try {
        logger->debug(QString("Получение списка пользователей (limit: %1, offset: %2)").arg(limit).arg(offset));
        return userRepository->getAllUsers(limit, offset);
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении списка пользователей: %1").arg(e.what()));
        return QList<User>();
    }
}

QList<User> AdminProcessor::searchUsers(const QString& searchTerm) {
    try {
        logger->debug(QString("Поиск пользователей по запросу: \"%1\"").arg(searchTerm));

        // Первичный поиск по имени пользователя
        QList<User> usersByName = userRepository->getUsersByUsername(searchTerm);

        // Если точное совпадение по email, добавляем его
        auto userByEmail = userRepository->getUserByEmail(searchTerm);
        if (userByEmail) {
            // Проверяем, нет ли уже такого пользователя в результатах
            bool alreadyExists = false;
            for (const auto& user : usersByName) {
                if (user.getId() == userByEmail->getId()) {
                    alreadyExists = true;
                    break;
                }
            }

            if (!alreadyExists) {
                usersByName.append(*userByEmail);
            }
        }

        logger->debug(QString("Найдено %1 пользователей").arg(usersByName.size()));
        return usersByName;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при поиске пользователей: %1").arg(e.what()));
        return QList<User>();
    }
}

QList<User> AdminProcessor::getUsersByRole(UserRole role) {
    try {
        logger->debug(QString("Получение пользователей с ролью: %1").arg(userRoleToString(role)));
        return userRepository->getUsersByRole(role);
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении пользователей по роли: %1").arg(e.what()));
        return QList<User>();
    }
}

std::optional<UserProfileDTO> AdminProcessor::getUserProfile(int userId) {
    try {
        logger->debug(QString("Получение профиля пользователя с ID: %1").arg(userId));

        // Получаем основные данные пользователя
        auto user = userRepository->getUserById(userId);
        if (!user) {
            logger->warn(QString("Пользователь с ID %1 не найден").arg(userId));
            return std::nullopt;
        }

        UserProfileDTO profile(*user);

        // Дополняем профиль данными о количестве избранных рецептов
        auto favorites = favoriteRepository->getFavoritesByUserId(userId);
        profile.favoriteRecipesCount = favorites.size();

        // Дополняем профиль данными о количестве отзывов
        auto reviews = reviewRepository->getReviewsByUserId(userId);
        profile.reviewsCount = reviews.size();

        return profile;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении профиля пользователя: %1").arg(e.what()));
        return std::nullopt;
    }
}

bool AdminProcessor::updateUserRole(int userId, UserRole role) {
    try {
        // Проверяем существование пользователя
        auto user = userRepository->getUserById(userId);
        if (!user) {
            logger->warn(QString("Попытка изменить роль несуществующего пользователя с ID %1").arg(userId));
            return false;
        }

        // Логируем действие
        logger->info(QString("Администратор изменяет роль пользователя %1 с '%2' на '%3'")
                         .arg(user->getUsername())
                         .arg(userRoleToString(user->getRole()))
                         .arg(userRoleToString(role)));

        // Обновляем роль пользователя
        bool success = userRepository->updateUserRole(userId, role);

        if (success) {
            logger->info(QString("Роль пользователя %1 успешно изменена на %2")
                             .arg(user->getUsername())
                             .arg(userRoleToString(role)));
        } else {
            logger->error(QString("Не удалось изменить роль пользователя %1")
                              .arg(user->getUsername()));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при обновлении роли пользователя: %1").arg(e.what()));
        return false;
    }
}

bool AdminProcessor::deleteUser(int userId) {
    try {
        // Проверяем существование пользователя
        auto user = userRepository->getUserById(userId);
        if (!user) {
            logger->warn(QString("Попытка удалить несуществующего пользователя с ID %1").arg(userId));
            return false;
        }

        // Не даем удалить собственного админа (это можно настроить по-другому)
        if (user->getRole() == UserRole::ADMIN && user->getUsername() == "admin") {
            logger->warn("Попытка удалить основного администратора системы");
            return false;
        }

        // Логируем действие
        logger->info(QString("Администратор удаляет пользователя: %1 (ID: %2)")
                         .arg(user->getUsername())
                         .arg(userId));

        // Удаляем пользователя
        bool success = userRepository->deleteUser(userId);

        if (success) {
            logger->info(QString("Пользователь %1 успешно удален").arg(user->getUsername()));
        } else {
            logger->error(QString("Не удалось удалить пользователя %1").arg(user->getUsername()));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при удалении пользователя: %1").arg(e.what()));
        return false;
    }
}

int AdminProcessor::getTotalUsersCount() {
    try {
        // Поскольку у IUserRepository нет метода для получения общего количества пользователей,
        // будем использовать метод getAllUsers с большим лимитом
        QList<User> allUsers = userRepository->getAllUsers(10000, 0);
        return allUsers.size();
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении общего количества пользователей: %1").arg(e.what()));
        return 0;
    }
}

int AdminProcessor::getUsersCountByRole(UserRole role) {
    try {
        QList<User> usersWithRole = userRepository->getUsersByRole(role);
        return usersWithRole.size();
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при получении количества пользователей с ролью %1: %2")
                          .arg(userRoleToString(role))
                          .arg(e.what()));
        return 0;
    }
}

// Методы для управления рецептами
bool AdminProcessor::updateRecipe(int recipeId, const QString& name, const QString& description, 
                                int preparationTime, int cookingTime, const QString& instructions) {
    try {
        // Проверяем существование рецепта
        auto recipe = recipeRepository->getRecipeById(recipeId);
        if (!recipe) {
            logger->warn(QString("Попытка обновить несуществующий рецепт с ID %1").arg(recipeId));
            return false;
        }

        // Логируем действие
        logger->info(QString("Администратор обновляет рецепт: %1 (ID: %2)")
                        .arg(recipe->getName())
                        .arg(recipeId));

        // Обновляем рецепт
        bool success = recipeRepository->updateRecipe(recipeId, name, description, 
                                                    preparationTime, cookingTime, instructions);

        if (success) {
            logger->info(QString("Рецепт %1 успешно обновлен").arg(name));
        } else {
            logger->error(QString("Не удалось обновить рецепт %1").arg(name));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при обновлении рецепта: %1").arg(e.what()));
        return false;
    }
}

bool AdminProcessor::deleteRecipe(int recipeId) {
    try {
        // Проверяем существование рецепта
        auto recipe = recipeRepository->getRecipeById(recipeId);
        if (!recipe) {
            logger->warn(QString("Попытка удалить несуществующий рецепт с ID %1").arg(recipeId));
            return false;
        }

        // Логируем действие
        logger->info(QString("Администратор удаляет рецепт: %1 (ID: %2)")
                        .arg(recipe->getName())
                        .arg(recipeId));

        // Удаляем рецепт
        bool success = recipeRepository->deleteRecipe(recipeId);

        if (success) {
            logger->info(QString("Рецепт %1 успешно удален").arg(recipe->getName()));
        } else {
            logger->error(QString("Не удалось удалить рецепт %1").arg(recipe->getName()));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при удалении рецепта: %1").arg(e.what()));
        return false;
    }
}

bool AdminProcessor::addCategoryToRecipe(int recipeId, int categoryId) {
    try {
        // Проверяем существование рецепта
        auto recipe = recipeRepository->getRecipeById(recipeId);
        if (!recipe) {
            logger->warn(QString("Попытка добавить категорию к несуществующему рецепту с ID %1").arg(recipeId));
            return false;
        }

        // Получаем категорию для логирования
        QString categoryName = "категория " + QString::number(categoryId);
        auto categories = recipeRepository->getAllCategories();
        for (const auto& category : categories) {
            if (category.getCategoryId() == categoryId) {
                categoryName = category.getName();
                break;
            }
        }

        // Логируем действие
        logger->info(QString("Администратор добавляет категорию '%1' к рецепту '%2' (ID: %3)")
                        .arg(categoryName)
                        .arg(recipe->getName())
                        .arg(recipeId));

        // Добавляем категорию к рецепту
        bool success = recipeRepository->addCategoryToRecipe(recipeId, categoryId);

        if (success) {
            logger->info(QString("Категория '%1' успешно добавлена к рецепту '%2'")
                            .arg(categoryName)
                            .arg(recipe->getName()));
        } else {
            logger->error(QString("Не удалось добавить категорию '%1' к рецепту '%2'")
                            .arg(categoryName)
                            .arg(recipe->getName()));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при добавлении категории к рецепту: %1").arg(e.what()));
        return false;
    }
}

bool AdminProcessor::removeCategoryFromRecipe(int recipeId, int categoryId) {
    try {
        // Проверяем существование рецепта
        auto recipe = recipeRepository->getRecipeById(recipeId);
        if (!recipe) {
            logger->warn(QString("Попытка удалить категорию у несуществующего рецепта с ID %1").arg(recipeId));
            return false;
        }

        // Получаем категорию для логирования
        QString categoryName = "категория " + QString::number(categoryId);
        auto categories = recipeRepository->getAllCategories();
        for (const auto& category : categories) {
            if (category.getCategoryId() == categoryId) {
                categoryName = category.getName();
                break;
            }
        }

        // Логируем действие
        logger->info(QString("Администратор удаляет категорию '%1' из рецепта '%2' (ID: %3)")
                        .arg(categoryName)
                        .arg(recipe->getName())
                        .arg(recipeId));

        // Удаляем категорию из рецепта
        bool success = recipeRepository->removeCategoryFromRecipe(recipeId, categoryId);

        if (success) {
            logger->info(QString("Категория '%1' успешно удалена из рецепта '%2'")
                            .arg(categoryName)
                            .arg(recipe->getName()));
        } else {
            logger->error(QString("Не удалось удалить категорию '%1' из рецепта '%2'")
                            .arg(categoryName)
                            .arg(recipe->getName()));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при удалении категории из рецепта: %1").arg(e.what()));
        return false;
    }
}

bool AdminProcessor::addIngredientToRecipe(int recipeId, int ingredientId, double quantity, 
                                        const QString& unit, bool isOptional) {
    try {
        // Проверяем существование рецепта
        auto recipe = recipeRepository->getRecipeById(recipeId);
        if (!recipe) {
            logger->warn(QString("Попытка добавить ингредиент к несуществующему рецепту с ID %1").arg(recipeId));
            return false;
        }

        // Логируем действие
        logger->info(QString("Администратор добавляет ингредиент (ID: %1) в количестве %2 %3 к рецепту '%4'")
                        .arg(ingredientId)
                        .arg(quantity)
                        .arg(unit)
                        .arg(recipe->getName()));

        // Добавляем ингредиент к рецепту
        bool success = recipeRepository->addIngredientToRecipe(recipeId, ingredientId, quantity, unit, isOptional);

        if (success) {
            logger->info(QString("Ингредиент (ID: %1) успешно добавлен к рецепту '%2'")
                            .arg(ingredientId)
                            .arg(recipe->getName()));
        } else {
            logger->error(QString("Не удалось добавить ингредиент (ID: %1) к рецепту '%2'")
                            .arg(ingredientId)
                            .arg(recipe->getName()));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при добавлении ингредиента к рецепту: %1").arg(e.what()));
        return false;
    }
}

bool AdminProcessor::removeIngredientFromRecipe(int recipeId, int ingredientId) {
    try {
        // Проверяем существование рецепта
        auto recipe = recipeRepository->getRecipeById(recipeId);
        if (!recipe) {
            logger->warn(QString("Попытка удалить ингредиент у несуществующего рецепта с ID %1").arg(recipeId));
            return false;
        }

        // Логируем действие
        logger->info(QString("Администратор удаляет ингредиент (ID: %1) из рецепта '%2'")
                        .arg(ingredientId)
                        .arg(recipe->getName()));

        // Удаляем ингредиент из рецепта
        bool success = recipeRepository->removeIngredientFromRecipe(recipeId, ingredientId);

        if (success) {
            logger->info(QString("Ингредиент (ID: %1) успешно удален из рецепта '%2'")
                            .arg(ingredientId)
                            .arg(recipe->getName()));
        } else {
            logger->error(QString("Не удалось удалить ингредиент (ID: %1) из рецепта '%2'")
                            .arg(ingredientId)
                            .arg(recipe->getName()));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при удалении ингредиента из рецепта: %1").arg(e.what()));
        return false;
    }
}

bool AdminProcessor::updateIngredientQuantity(int recipeId, int ingredientId, double quantity) {
    try {
        // Проверяем существование рецепта
        auto recipe = recipeRepository->getRecipeById(recipeId);
        if (!recipe) {
            logger->warn(QString("Попытка обновить количество ингредиента у несуществующего рецепта с ID %1").arg(recipeId));
            return false;
        }

        // Логируем действие
        logger->info(QString("Администратор обновляет количество ингредиента (ID: %1) до %2 в рецепте '%3'")
                        .arg(ingredientId)
                        .arg(quantity)
                        .arg(recipe->getName()));

        // Обновляем количество ингредиента
        bool success = recipeRepository->updateIngredientQuantity(recipeId, ingredientId, quantity);

        if (success) {
            logger->info(QString("Количество ингредиента (ID: %1) успешно обновлено в рецепте '%2'")
                            .arg(ingredientId)
                            .arg(recipe->getName()));
        } else {
            logger->error(QString("Не удалось обновить количество ингредиента (ID: %1) в рецепте '%2'")
                            .arg(ingredientId)
                            .arg(recipe->getName()));
        }

        return success;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при обновлении количества ингредиента в рецепте: %1").arg(e.what()));
        return false;
    }
}

int AdminProcessor::createRecipe(const QString& name, const QString& description,
                                 int preparationTime, int cookingTime,
                                 const QString& instructions, int authorId) {
    try {
        if (name.trimmed().isEmpty()) {
            logger->warn("Попытка создать рецепт с пустым названием");
            return -1;
        }

        // Логируем действие
        logger->info(QString("Администратор создает новый рецепт: %1").arg(name));

        // Создаем рецепт
        int newRecipeId = recipeRepository->createRecipe(name, description,
                                                         preparationTime, cookingTime,
                                                         instructions, authorId);

        if (newRecipeId > 0) {
            logger->info(QString("Рецепт '%1' успешно создан с ID: %2").arg(name).arg(newRecipeId));
        } else {
            logger->error(QString("Не удалось создать рецепт '%1'").arg(name));
        }

        return newRecipeId;
    } catch (const std::exception& e) {
        logger->error(QString("Ошибка при создании рецепта: %1").arg(e.what()));
        return -1;
    }
}

