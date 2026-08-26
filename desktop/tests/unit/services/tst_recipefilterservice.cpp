// tests/unit/services/tst_recipefilterservice.cpp
#include <QtTest>
#include "application/services/recipeFilterService.h"

class RecipeFilterServiceTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testFilterByTime();
    void testFilterByCategories();
    void testFilterRecipes();
    void cleanupTestCase();

private:
    RecipeFilterService filterService;
    QList<Recipe> testRecipes;

    // Вспомогательная функция для создания тестовых рецептов
    Recipe createTestRecipe(int id, const QString& name, int prepTime, int cookTime,
                            const QList<int>& categoryIds, const QList<RecipeIngredient>& ingredients);
};

void RecipeFilterServiceTest::initTestCase() {
    // Создаем тестовые ингредиенты
    RecipeIngredient ing1(1, 100, 200.0, "г", false); // Ингредиент 1 (обязательный)
    RecipeIngredient ing2(1, 101, 300.0, "г", false); // Ингредиент 2 (обязательный)
    RecipeIngredient ing3(1, 102, 150.0, "г", true);  // Ингредиент 3 (опциональный)
    RecipeIngredient ing4(1, 103, 50.0, "мл", false); // Ингредиент 4 (обязательный)
    RecipeIngredient ing5(1, 104, 2.0, "шт", false);  // Ингредиент 5 (обязательный)

    // Создаем несколько тестовых рецептов с разными характеристиками
    testRecipes.append(createTestRecipe(1, "Рецепт 1", 15, 30, {1, 2}, {ing1, ing2}));
    testRecipes.append(createTestRecipe(2, "Рецепт 2", 10, 45, {2, 3}, {ing1, ing3, ing4}));
    testRecipes.append(createTestRecipe(3, "Рецепт 3", 30, 60, {1, 4}, {ing2, ing5}));
    testRecipes.append(createTestRecipe(4, "Рецепт 4", 5, 15, {3}, {ing3, ing5}));
    testRecipes.append(createTestRecipe(5, "Рецепт 5", 25, 75, {1, 2, 3}, {ing1, ing2, ing3, ing4, ing5}));
}

Recipe RecipeFilterServiceTest::createTestRecipe(int id, const QString& name, int prepTime, int cookTime,
                                                 const QList<int>& categoryIds, const QList<RecipeIngredient>& ingredients) {
    QStringList steps = {"Шаг 1", "Шаг 2", "Шаг 3"};
    return Recipe(id, name, "Описание рецепта " + QString::number(id), prepTime, cookTime,
                  categoryIds, steps, ingredients);
}

void RecipeFilterServiceTest::testFilterByTime() {
    // Тест фильтрации по времени приготовления
    QList<Recipe> filteredByPrepTime = filterService.filterByTime(testRecipes, 20, -1);
    QCOMPARE(filteredByPrepTime.size(), 3); // Рецепты 1, 2, 4 имеют время подготовки <= 20

    // Проверка ID рецептов
    QSet<int> expectedPrepIds = {1, 2, 4};
    QSet<int> actualPrepIds;
    for (const Recipe& recipe : filteredByPrepTime) {
        actualPrepIds.insert(recipe.getRecipeId());
    }
    QCOMPARE(actualPrepIds, expectedPrepIds);

    // Тест фильтрации по времени готовки
    QList<Recipe> filteredByCookTime = filterService.filterByTime(testRecipes, -1, 30);
    QCOMPARE(filteredByCookTime.size(), 2); // Рецепты 1, 4 имеют время готовки <= 30

    // Проверка ID рецептов
    QSet<int> expectedCookIds = {1, 4};
    QSet<int> actualCookIds;
    for (const Recipe& recipe : filteredByCookTime) {
        actualCookIds.insert(recipe.getRecipeId());
    }
    QCOMPARE(actualCookIds, expectedCookIds);

    // Тест фильтрации по обоим временам
    QList<Recipe> filteredByBothTimes = filterService.filterByTime(testRecipes, 15, 45);
    QCOMPARE(filteredByBothTimes.size(), 3); // Рецепты 1, 2, 4 подходят под оба условия

    // Проверка ID рецептов
    QSet<int> expectedBothIds = {1, 2, 4};
    QSet<int> actualBothIds;
    for (const Recipe& recipe : filteredByBothTimes) {
        actualBothIds.insert(recipe.getRecipeId());
    }
    QCOMPARE(actualBothIds, expectedBothIds);
}

void RecipeFilterServiceTest::testFilterByCategories() {
    // Тест фильтрации по одной категории
    QList<Recipe> filteredByCat1 = filterService.filterByCategories(testRecipes, {1});
    QCOMPARE(filteredByCat1.size(), 3); // Рецепты 1, 3, 5 имеют категорию 1

    // Проверка ID рецептов
    QSet<int> expectedCat1Ids = {1, 3, 5};
    QSet<int> actualCat1Ids;
    for (const Recipe& recipe : filteredByCat1) {
        actualCat1Ids.insert(recipe.getRecipeId());
    }
    QCOMPARE(actualCat1Ids, expectedCat1Ids);

    // Тест фильтрации по нескольким категориям
    QList<Recipe> filteredByCats23 = filterService.filterByCategories(testRecipes, {2, 3});
    QCOMPARE(filteredByCats23.size(), 4); // Рецепты 1, 2, 4, 5 имеют категорию 2 или 3

    // Проверка ID рецептов
    QSet<int> expectedCats23Ids = {1, 2, 4, 5};
    QSet<int> actualCats23Ids;
    for (const Recipe& recipe : filteredByCats23) {
        actualCats23Ids.insert(recipe.getRecipeId());
    }
    QCOMPARE(actualCats23Ids, expectedCats23Ids);

    // Тест фильтрации по несуществующей категории
    QList<Recipe> filteredByNonExistentCat = filterService.filterByCategories(testRecipes, {999});
    QVERIFY(filteredByNonExistentCat.isEmpty());
}

void RecipeFilterServiceTest::testFilterRecipes() {
    // Создаем фильтр для тестирования
    RecipeFilterDTO filter;

    // Тест с фильтром по времени
    filter.setMaxPrepTime(20);
    filter.setMaxCookingTime(30);
    QList<Recipe> filteredByTime = filterService.filterRecipes(testRecipes, filter);
    QCOMPARE(filteredByTime.size(), 2); // Рецепты 1, 4

    // Тест с фильтром по категориям
    RecipeFilterDTO catFilter;
    QList<int> categoryIds = {1};
    catFilter.setCategoryIds(categoryIds);
    QList<Recipe> filteredByCat = filterService.filterRecipes(testRecipes, catFilter);
    QCOMPARE(filteredByCat.size(), 3); // Рецепты 1, 3, 5

    // Тест с комбинированным фильтром
    RecipeFilterDTO combinedFilter;
    combinedFilter.setMaxPrepTime(20);
    combinedFilter.setCategoryIds(categoryIds);
    QList<Recipe> filteredCombined = filterService.filterRecipes(testRecipes, combinedFilter);
    QCOMPARE(filteredCombined.size(), 1); // Только рецепт 1

    // Проверяем ID рецепта
    QCOMPARE(filteredCombined.first().getRecipeId(), 1);
}

void RecipeFilterServiceTest::cleanupTestCase() {
    // Очистка ресурсов, если необходимо
    testRecipes.clear();
}

// QTEST_APPLESS_MAIN(RecipeFilterServiceTest)
#include "tst_recipefilterservice.moc"
