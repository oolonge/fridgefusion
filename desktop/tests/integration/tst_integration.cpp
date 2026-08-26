#include <QtTest>
#include "infrastructure/persistence/database/dbContext.h"
#include "infrastructure/persistence/postgresql/userRepository.h"
#include "infrastructure/persistence/postgresql/recipeRepository.h"
#include "infrastructure/persistence/postgresql/ingredientRepository.h"
#include "infrastructure/persistence/postgresql/favoriteRepository.h"
#include "infrastructure/persistence/postgresql/reviewRepository.h"
#include "application/processors/authProcessor.h"
#include "application/processors/recipeExplorerProcessor.h"
#include "application/processors/favoriteProcessor.h"
#include "application/processors/reviewProcessor.h"
#include "application/services/recipeFilterService.h"
#include "application/services/passwordHasher.h"
#include <memory>

class FridgeFusionIntegrationTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testUserRegistrationAndAuthentication();
    void testRecipeSearchByIngredients();
    void testFavoriteRecipesManagement();
    void testReviewsManagement();
    void cleanupTestCase();

private:
    std::shared_ptr<DbContext> dbContext;

    std::shared_ptr<UserRepository> userRepository;
    std::shared_ptr<RecipeRepository> recipeRepository;
    std::shared_ptr<IngredientRepository> ingredientRepository;
    std::shared_ptr<FavoriteRepository> favoriteRepository;
    std::shared_ptr<ReviewRepository> reviewRepository;

    std::shared_ptr<PasswordHasher> passwordHasher;
    std::shared_ptr<RecipeFilterService> recipeFilterService;

    std::shared_ptr<AuthProcessor> authProcessor;
    std::shared_ptr<RecipeExplorerProcessor> recipeExplorerProcessor;
    std::shared_ptr<FavoriteProcessor> favoriteProcessor;
    std::shared_ptr<ReviewProcessor> reviewProcessor;

    // Данные для тестов
    int testUserId = -1;
    QString testUserEmail = "integration_test@example.com";
    QList<int> testRecipeIds;
};

void FridgeFusionIntegrationTest::initTestCase() {
    dbContext = std::make_shared<DbContext>("integration_test");
    QVERIFY(dbContext->connect());

    userRepository = std::make_shared<UserRepository>(dbContext);
    recipeRepository = std::make_shared<RecipeRepository>(dbContext);
    ingredientRepository = std::make_shared<IngredientRepository>(dbContext);
    favoriteRepository = std::make_shared<FavoriteRepository>(dbContext.get());
    reviewRepository = std::make_shared<ReviewRepository>(dbContext.get());

    passwordHasher = std::make_shared<PasswordHasher>();
    recipeFilterService = std::make_shared<RecipeFilterService>();

    authProcessor = std::make_shared<AuthProcessor>(userRepository, passwordHasher);
    recipeExplorerProcessor = std::make_shared<RecipeExplorerProcessor>(
        recipeRepository, ingredientRepository, recipeFilterService);
    favoriteProcessor = std::make_shared<FavoriteProcessor>(favoriteRepository.get());
    reviewProcessor = std::make_shared<ReviewProcessor>(reviewRepository.get());

    QList<Recipe> recipes = recipeRepository->getAllRecipes(5, 0);
    QVERIFY(!recipes.isEmpty());

    for (const Recipe& recipe : recipes) {
        testRecipeIds.append(recipe.getRecipeId());
    }
}

void FridgeFusionIntegrationTest::testUserRegistrationAndAuthentication() {
    qDebug() << "Тестирование регистрации и аутентификации пользователя";

    // 1. Регистрация
    QString uniqueSuffix = QString::number(QDateTime::currentSecsSinceEpoch());
    QString testUsername = "integration_user_" + uniqueSuffix;
    QString testEmail = "integration_test_" + uniqueSuffix + "@example.com";
    QString testPassword = "test_password_123";

    AuthResult registrationResult = authProcessor->registerUser(testUsername, testEmail, testPassword);
    QVERIFY(registrationResult.isSuccess());
    QVERIFY(registrationResult.getUser().has_value());
    QCOMPARE(registrationResult.getUser()->getUsername(), testUsername);
    QCOMPARE(registrationResult.getUser()->getEmail(), testEmail);

    testUserId = registrationResult.getUser()->getId();
    testUserEmail = testEmail;

    qDebug() << "Пользователь успешно зарегистрирован с ID:" << testUserId;

    // 2. Выход
    QVERIFY(authProcessor->logout());
    QVERIFY(!authProcessor->isUserLoggedIn());

    // 3. Вход
    AuthResult loginResult = authProcessor->login(testEmail, testPassword);
    QVERIFY(loginResult.isSuccess());
    QVERIFY(loginResult.getUser().has_value());
    QCOMPARE(loginResult.getUser()->getEmail(), testEmail);
    QVERIFY(authProcessor->isUserLoggedIn());

    qDebug() << "Пользователь успешно вошел в систему";
}

void FridgeFusionIntegrationTest::testRecipeSearchByIngredients() {
    qDebug() << "Тестирование поиска рецептов по ингредиентам";

    // 1. Получение нескольких случайных ингредиентов
    QList<Ingredient> allIngredients = ingredientRepository->getAllIngredients();
    QVERIFY(!allIngredients.isEmpty());

    // Случайно выбираем 2-3 ингредиента
    QList<int> selectedIngredientIds;
    int ingredientsCount = qMin(3, allIngredients.size());

    for (int i = 0; i < ingredientsCount; i++) {
        int randomIndex = QRandomGenerator::global()->bounded(allIngredients.size());
        selectedIngredientIds.append(allIngredients[randomIndex].getIngredientId());
    }

    qDebug() << "Выбранные ID ингредиентов:" << selectedIngredientIds;

    // 2. Поиск рецептов по выбранным ингредиентам
    QList<Recipe> foundRecipes = recipeExplorerProcessor->findRecipesWithIngredients(selectedIngredientIds);

    qDebug() << "Найдено рецептов:" << foundRecipes.size();
    for (const Recipe& recipe : foundRecipes) {
        qDebug() << "  ID:" << recipe.getRecipeId() << "Название:" << recipe.getName();
    }

    // 3. Проверка получения деталей рецепта
    if (!foundRecipes.isEmpty()) {
        int recipeId = foundRecipes.first().getRecipeId();
        RecipeDetailDTO recipeDetail = recipeExplorerProcessor->getRecipeDetail(recipeId);

        QCOMPARE(recipeDetail.id, recipeId);
        QVERIFY(!recipeDetail.name.isEmpty());
        QVERIFY(!recipeDetail.ingredients.isEmpty());

        qDebug() << "Успешно получены детали рецепта:" << recipeDetail.name;
    }

    // 4. Поиск рецептов по категории
    int categoryId = 1; // Предполагается, что категория с ID 1 существует
    QList<Recipe> recipesByCategory = recipeExplorerProcessor->findRecipesByCategory(categoryId);

    qDebug() << "Найдено рецептов по категории:" << recipesByCategory.size();
}

void FridgeFusionIntegrationTest::testFavoriteRecipesManagement() {
    qDebug() << "Тестирование управления избранными рецептами";

    QVERIFY(testUserId > 0);
    QVERIFY(!testRecipeIds.isEmpty());

    int recipeId = testRecipeIds.first();

    // 1. Добавление рецепта в избранное
    bool addResult = favoriteProcessor->addToFavorites(testUserId, recipeId);
    QVERIFY(addResult);

    // 2. Проверка наличия рецепта в избранном
    bool isFavorite = favoriteProcessor->isRecipeFavorite(testUserId, recipeId);
    QVERIFY(isFavorite);

    // 3. Получение списка избранных рецептов
    QList<Favorite> userFavorites = favoriteProcessor->getUserFavorites(testUserId);
    QVERIFY(!userFavorites.isEmpty());

    bool foundAddedFavorite = false;
    for (const Favorite& favorite : userFavorites) {
        if (favorite.getRecipeId() == recipeId) {
            foundAddedFavorite = true;
            break;
        }
    }
    QVERIFY(foundAddedFavorite);

    qDebug() << "Рецепт успешно добавлен в избранное";

    // 4. Удаление рецепта из избранного
    bool removeResult = favoriteProcessor->removeFromFavorites(testUserId, recipeId);
    QVERIFY(removeResult);

    // 5. Проверка отсутствия рецепта в избранном после удаления
    isFavorite = favoriteProcessor->isRecipeFavorite(testUserId, recipeId);
    QVERIFY(!isFavorite);

    qDebug() << "Рецепт успешно удален из избранного";
}

void FridgeFusionIntegrationTest::testReviewsManagement() {
    qDebug() << "Тестирование управления отзывами";

    QVERIFY(testUserId > 0);
    QVERIFY(!testRecipeIds.isEmpty());

    int recipeId = testRecipeIds.last();

    // 1. Добавление отзыва
    QString testComment = "Отличный рецепт, всем рекомендую!";
    int testRating = 5;

    bool addResult = reviewProcessor->addReview(testUserId, recipeId, testRating, testComment);
    QVERIFY(addResult);

    qDebug() << "Отзыв успешно добавлен";

    // 2. Получение отзывов пользователя
    QList<Review> userReviews = reviewProcessor->getUserReviews(testUserId);
    QVERIFY(!userReviews.isEmpty());

    Review addedReview;
    bool foundAddedReview = false;

    for (const Review& review : userReviews) {
        if (review.getRecipeId() == recipeId) {
            addedReview = review;
            foundAddedReview = true;
            break;
        }
    }

    QVERIFY(foundAddedReview);
    QCOMPARE(addedReview.getRating(), testRating);
    QCOMPARE(addedReview.getComment(), testComment);

    // 3. Обновление отзыва
    int updatedRating = 4;
    QString updatedComment = "Хороший рецепт, но немного сложный в приготовлении";

    bool updateResult = reviewProcessor->updateReview(addedReview.getId(), updatedRating, updatedComment);
    QVERIFY(updateResult);

    qDebug() << "Отзыв успешно обновлен";

    // 4. Получение отзывов для рецепта
    QList<Review> recipeReviews = reviewProcessor->getRecipeReviews(recipeId);
    QVERIFY(!recipeReviews.isEmpty());

    bool foundUpdatedReview = false;

    for (const Review& review : recipeReviews) {
        if (review.getId() == addedReview.getId()) {
            QCOMPARE(review.getRating(), updatedRating);
            QCOMPARE(review.getComment(), updatedComment);
            foundUpdatedReview = true;
            break;
        }
    }

    QVERIFY(foundUpdatedReview);

    // 5. Получение среднего рейтинга для рецепта
    double avgRating = reviewProcessor->getRecipeAverageRating(recipeId);
    QVERIFY(avgRating > 0);

    qDebug() << "Средний рейтинг рецепта:" << avgRating;

    // 6. Удаление отзыва
    bool deleteResult = reviewProcessor->deleteReview(addedReview.getId());
    QVERIFY(deleteResult);

    qDebug() << "Отзыв успешно удален";
}

void FridgeFusionIntegrationTest::cleanupTestCase() {
    qDebug() << "Очистка тестовых данных";

    // Удаление тестового пользователя, если он был создан
    if (testUserId > 0) {
        try {
            userRepository->deleteUser(testUserId);
            qDebug() << "Тестовый пользователь удален";
        } catch (const std::exception& e) {
            qDebug() << "Ошибка при удалении тестового пользователя:" << e.what();
        }
    }

    // Закрытие соединения с БД
    if (dbContext && dbContext->isConnected()) {
        dbContext->disconnect();
    }
}

// QTEST_MAIN(FridgeFusionIntegrationTest)
#include "tst_integration.moc"
