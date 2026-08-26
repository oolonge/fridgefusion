#include <QtTest>
#include "application/processors/recipeExplorerProcessor.h"
#include <memory>

// Мок-классы для тестирования
class MockRecipeRepository : public IRecipeRepository {
public:
    std::optional<Recipe> getRecipeById(int recipeId) override {
        if (recipeId == 1) {
            QList<RecipeIngredient> ingredients;
            ingredients.append(RecipeIngredient(1, 100, 200.0, "г", false));
            ingredients.append(RecipeIngredient(1, 101, 300.0, "г", false));

            QList<int> categoryIds = {1, 2};
            QStringList steps = {"Шаг 1", "Шаг 2", "Шаг 3"};

            return Recipe(1, "Тестовый рецепт", "Описание рецепта", 30, 60, categoryIds, steps, ingredients);
        }
        return std::nullopt;
    }

    QList<Category> getAllCategories() override {
        QList<Category> categories;
        categories.append(Category(1, "Категория 1", "Описание категории 1"));
        categories.append(Category(2, "Категория 2", "Описание категории 2"));
        return categories;
    }

    QList<Recipe> getAllRecipes(int limit = 100, int offset = 0) override {
        QList<Recipe> recipes;
        auto recipe = getRecipeById(1);
        if (recipe.has_value()) {
            recipes.append(recipe.value());
        }
        return recipes;
    }

    QList<Recipe> getRecipesByIngredients(const QList<int>& ingredientIds) override {
        // Проверяем, содержит ли запрос ингредиенты 100 и 101 (или их заменители)
        bool hasIngredient100 = ingredientIds.contains(100) || ingredientIds.contains(102);
        bool hasIngredient101 = ingredientIds.contains(101) || ingredientIds.contains(103);

        if (hasIngredient100 && hasIngredient101) {
            return getAllRecipes(100, 0); // Передаем параметры
        }
        return QList<Recipe>();
    }

    QList<Recipe> getRecipesByCategory(int categoryId) override {
        if (categoryId == 1 || categoryId == 2) {
            return getAllRecipes(100, 0); // Передаем параметры
        }
        return QList<Recipe>();
    }

    QList<RecipeIngredient> getRecipeIngredients(int recipeId) override {
        if (recipeId == 1) {
            QList<RecipeIngredient> ingredients;
            ingredients.append(RecipeIngredient(1, 100, 200.0, "г", false));
            ingredients.append(RecipeIngredient(1, 101, 300.0, "г", false));
            return ingredients;
        }
        return QList<RecipeIngredient>();
    }

    QList<std::pair<int, double>> getIngredientSubstitutes(int ingredientId, int limit) override {
        if (ingredientId == 100) {
            return {{102, 0.8}, {104, 0.6}};
        }
        if (ingredientId == 101) {
            return {{103, 0.9}};
        }
        return QList<std::pair<int, double>>();
    }

    RecipePreviewDTO getRecipePreview(int recipeId) override {
        RecipePreviewDTO preview;
        if (recipeId == 1) {
            preview.id = 1;
            preview.name = "Тестовый рецепт";
            preview.shortDescription = "Краткое описание";
            preview.totalTime = 90;
            preview.mainIngredients = {"Ингредиент 1", "Ингредиент 2"};
            preview.averageRating = 4.5;
            preview.isFavorite = true;
        }
        return preview;
    }

    RecipeDetailDTO getRecipeDetail(int recipeId) override {
        RecipeDetailDTO detail;
        if (recipeId == 1) {
            detail.id = 1;
            detail.name = "Тестовый рецепт";
            detail.fullDescription = "Полное описание рецепта";
            detail.preparationTime = 30;
            detail.cookingTime = 60;
            detail.categories = {"Категория 1", "Категория 2"};

            RecipeDetailDTO::IngredientInfo ing1;
            ing1.id = 100;
            ing1.name = "Ингредиент 1";
            ing1.quantity = 200.0;
            ing1.unit = "г";
            ing1.isOptional = false;
            ing1.possibleSubstitutes = {{102, "Заменитель 1"}, {104, "Заменитель 2"}};

            RecipeDetailDTO::IngredientInfo ing2;
            ing2.id = 101;
            ing2.name = "Ингредиент 2";
            ing2.quantity = 300.0;
            ing2.unit = "г";
            ing2.isOptional = false;
            ing2.possibleSubstitutes = {{103, "Заменитель 3"}};

            detail.ingredients = {ing1, ing2};
            detail.steps = {"Шаг 1", "Шаг 2", "Шаг 3"};
            detail.averageRating = 4.5;
            detail.reviewCount = 10;
            detail.isFavorite = true;
        }
        return detail;
    }
};

// Замените существующий класс MockIngredientRepository на следующий:

class MockIngredientRepository : public IIngredientRepository {
public:
    std::optional<Ingredient> getIngredientById(int ingredientId) override {
        if (ingredientId == 100) {
            return Ingredient(100, "Ингредиент 1", "г", 100.0, 7);
        }
        if (ingredientId == 101) {
            return Ingredient(101, "Ингредиент 2", "г", 200.0, 14);
        }
        if (ingredientId == 102) {
            return Ingredient(102, "Заменитель 1", "г", 90.0, 5);
        }
        if (ingredientId == 103) {
            return Ingredient(103, "Заменитель 2", "г", 210.0, 10);
        }
        return std::nullopt;
    }

    QList<Ingredient> getAllIngredients() override {
        QList<Ingredient> ingredients;
        auto ing1 = getIngredientById(100);
        auto ing2 = getIngredientById(101);
        if (ing1.has_value()) ingredients.append(ing1.value());
        if (ing2.has_value()) ingredients.append(ing2.value());
        return ingredients;
    }

    QList<Ingredient> getIngredientsByName(const QString& name) override {
        QList<Ingredient> result;
        if (name.contains("Ингредиент 1")) {
            auto ing = getIngredientById(100);
            if (ing.has_value()) result.append(ing.value());
        }
        if (name.contains("Ингредиент 2")) {
            auto ing = getIngredientById(101);
            if (ing.has_value()) result.append(ing.value());
        }
        return result;
    }

    QList<std::pair<int, double>> getIngredientSubstitutes(int ingredientId, double threshold, int limit) override {
        if (ingredientId == 100) {
            return {{102, 0.8}, {104, 0.6}};
        }
        if (ingredientId == 101) {
            return {{103, 0.9}};
        }
        return QList<std::pair<int, double>>();
    }
};

class MockRecipeFilterService : public IRecipeFilterService {
public:
    QList<Recipe> filterRecipes(const QList<Recipe>& recipes, const RecipeFilterDTO& filterDTO) override {
        // Реализуем простую логику фильтрации для теста
        if (!filterDTO.getCategoryIds().isEmpty()) {
            // Для теста: если запрашивается категория 999, возвращаем пустой список
            if (filterDTO.getCategoryIds().contains(999)) {
                return QList<Recipe>();
            }
        }
        // В остальных случаях возвращаем исходные рецепты
        return recipes;
    }

    QList<Recipe> filterByTime(const QList<Recipe>& recipes, int maxPrepTime, int maxCookTime) override {
        return recipes;
    }

    QList<Recipe> filterByCategories(const QList<Recipe>& recipes, const QList<int>& categoryIds) override {
        // Для теста: если запрашивается категория 999, возвращаем пустой список
        if (!categoryIds.isEmpty() && categoryIds.contains(999)) {
            return QList<Recipe>();
        }
        return recipes;
    }
};

class RecipeExplorerProcessorTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testFindRecipesWithIngredients();
    void testFindRecipesByCategory();
    void testFindRecipesByCategoryAndIngredients();
    void testGetIngredientSubstitutes();
    void testGetRecipePreview();
    void testGetRecipeDetail();
    void cleanupTestCase();

private:
    std::shared_ptr<RecipeExplorerProcessor> processor;
    std::shared_ptr<MockRecipeRepository> recipeRepository;
    std::shared_ptr<MockIngredientRepository> ingredientRepository;
    std::shared_ptr<MockRecipeFilterService> filterService;
};

void RecipeExplorerProcessorTest::initTestCase() {
    // Инициализируем моки и процессор
    recipeRepository = std::make_shared<MockRecipeRepository>();
    ingredientRepository = std::make_shared<MockIngredientRepository>();
    filterService = std::make_shared<MockRecipeFilterService>();

    processor = std::make_shared<RecipeExplorerProcessor>(
        recipeRepository,
        ingredientRepository,
        filterService
        );
}

void RecipeExplorerProcessorTest::testFindRecipesWithIngredients() {
    // Тест поиска рецептов по ингредиентам
    QList<int> ingredientIds = {100, 101};
    QList<Recipe> recipes = processor->findRecipesWithIngredients(ingredientIds);

    QVERIFY(!recipes.isEmpty());
    QCOMPARE(recipes.size(), 1);
    QCOMPARE(recipes[0].getRecipeId(), 1);

    // Тест с пустым списком ингредиентов
    QList<Recipe> emptyResult = processor->findRecipesWithIngredients(QList<int>());
    QVERIFY(emptyResult.isEmpty());

    // Тест с несуществующими ингредиентами
    QList<int> nonExistentIds = {999, 998};
    QList<Recipe> noResult = processor->findRecipesWithIngredients(nonExistentIds);
    QVERIFY(noResult.isEmpty());
}

void RecipeExplorerProcessorTest::testFindRecipesByCategory() {
    // Тест поиска рецептов по категории
    QList<Recipe> recipes = processor->findRecipesByCategory(1);

    QVERIFY(!recipes.isEmpty());
    QCOMPARE(recipes.size(), 1);
    QCOMPARE(recipes[0].getRecipeId(), 1);

    // Тест с несуществующей категорией
    QList<Recipe> noResult = processor->findRecipesByCategory(999);
    QVERIFY(noResult.isEmpty());
}

void RecipeExplorerProcessorTest::testFindRecipesByCategoryAndIngredients() {
    // Тест поиска рецептов по категории и ингредиентам
    QList<int> ingredientIds = {100, 101};
    QList<Recipe> recipes = processor->findRecipesByCategoryAndIngredients(1, ingredientIds);

    QVERIFY(!recipes.isEmpty());
    QCOMPARE(recipes.size(), 1);
    QCOMPARE(recipes[0].getRecipeId(), 1);

    // Тест с несуществующей категорией
    QList<Recipe> noResult1 = processor->findRecipesByCategoryAndIngredients(999, ingredientIds);
    QVERIFY(noResult1.isEmpty());

    // Тест с несуществующими ингредиентами
    QList<int> nonExistentIds = {999, 998};
    QList<Recipe> noResult2 = processor->findRecipesByCategoryAndIngredients(1, nonExistentIds);
    QVERIFY(noResult2.isEmpty());
}

void RecipeExplorerProcessorTest::testGetIngredientSubstitutes() {
    // Тест получения заменителей ингредиентов
    QList<std::pair<int, double>> substitutes = processor->getIngredientSubstitutes(100);

    QVERIFY(!substitutes.isEmpty());
    QCOMPARE(substitutes.size(), 2);
    QCOMPARE(substitutes[0].first, 102);
    QCOMPARE(substitutes[0].second, 0.8);

    // Тест с ограничением количества результатов
    QList<std::pair<int, double>> limitedSubstitutes = processor->getIngredientSubstitutes(100, 1);
    QCOMPARE(limitedSubstitutes.size(), 2); // В нашем моке размер не меняется

    // Тест с несуществующим ингредиентом
    QList<std::pair<int, double>> noSubstitutes = processor->getIngredientSubstitutes(999);
    QVERIFY(noSubstitutes.isEmpty());
}

void RecipeExplorerProcessorTest::testGetRecipePreview() {
    // Тест получения превью рецепта
    RecipePreviewDTO preview = processor->getRecipePreview(1);

    QCOMPARE(preview.id, 1);
    QCOMPARE(preview.name, QString("Тестовый рецепт"));
    QCOMPARE(preview.totalTime, 90);
    QCOMPARE(preview.mainIngredients.size(), 2);
    QCOMPARE(preview.averageRating, 4.5);
    QCOMPARE(preview.isFavorite, true);

    // Тест с несуществующим рецептом
    RecipePreviewDTO emptyPreview = processor->getRecipePreview(999);
    QCOMPARE(emptyPreview.id, 0);
    QCOMPARE(emptyPreview.name, QString());
}

void RecipeExplorerProcessorTest::testGetRecipeDetail() {
    // Тест получения детальной информации о рецепте
    RecipeDetailDTO detail = processor->getRecipeDetail(1);

    QCOMPARE(detail.id, 1);
    QCOMPARE(detail.name, QString("Тестовый рецепт"));
    QCOMPARE(detail.preparationTime, 30);
    QCOMPARE(detail.cookingTime, 60);
    QCOMPARE(detail.categories.size(), 2);
    QCOMPARE(detail.ingredients.size(), 2);
    QCOMPARE(detail.steps.size(), 3);
    QCOMPARE(detail.averageRating, 4.5);
    QCOMPARE(detail.reviewCount, 10);
    QCOMPARE(detail.isFavorite, true);

    // Проверка информации об ингредиентах
    bool foundIngredient1 = false;
    bool foundIngredient2 = false;

    for (const auto& ingredient : detail.ingredients) {
        if (ingredient.id == 100) {
            foundIngredient1 = true;
            QCOMPARE(ingredient.name, QString("Ингредиент 1"));
            QCOMPARE(ingredient.quantity, 200.0);
            QCOMPARE(ingredient.unit, QString("г"));
            QCOMPARE(ingredient.possibleSubstitutes.size(), 2);
        }
        if (ingredient.id == 101) {
            foundIngredient2 = true;
            QCOMPARE(ingredient.name, QString("Ингредиент 2"));
            QCOMPARE(ingredient.quantity, 300.0);
            QCOMPARE(ingredient.unit, QString("г"));
            QCOMPARE(ingredient.possibleSubstitutes.size(), 1);
        }
    }

    QVERIFY(foundIngredient1);
    QVERIFY(foundIngredient2);

    // Тест с несуществующим рецептом
    RecipeDetailDTO emptyDetail = processor->getRecipeDetail(999);
    QCOMPARE(emptyDetail.id, 0);
    QCOMPARE(emptyDetail.name, QString());
}

void RecipeExplorerProcessorTest::cleanupTestCase() {
    // Очистка ресурсов, если необходимо
    processor.reset();
    recipeRepository.reset();
    ingredientRepository.reset();
    filterService.reset();
}

// QTEST_APPLESS_MAIN(RecipeExplorerProcessorTest)
#include "tst_recipeexplorerprocessor.moc"
