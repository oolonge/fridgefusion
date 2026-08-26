#include <QtTest>
#include <QCoreApplication>
#include <QDebug>

// Сервисы
#include "unit/services/tst_passwordhasher.cpp"
#include "unit/services/tst_ingredientservice.cpp"
#include "unit/services/tst_recipefilterservice.cpp"
#include "unit/services/tst_recipesortingservice.cpp"

// Процессоры
#include "unit/processors/tst_authprocessor.cpp"
#include "unit/processors/tst_userprocessor.cpp"
#include "unit/processors/tst_recipeexplorerprocessor.cpp"
#include "unit/processors/tst_favoriteprocessor.cpp"
#include "unit/processors/tst_reviewprocessor.cpp"

// Интеграционные тесты
#include "integration/tst_dbconnection.cpp"
#include "integration/tst_integration.cpp"


int main(int argc, char *argv[])
{
    // Инициализируем приложение перед любым использованием Qt SQL
    QCoreApplication app(argc, argv);

    // Путь к плагинам SQL
    QCoreApplication::addLibraryPath("/opt/homebrew/Cellar/qt-postgresql/6.9.0/lib");

    // Проверка и вывод доступных SQL драйверов
    qDebug() << "Доступные SQL драйверы:";
    qDebug() << QSqlDatabase::drivers();

    int status = 0;

    // {
    //     PasswordHasherTest passwordHasherTest;
    //     status |= QTest::qExec(&passwordHasherTest, argc, argv);
    // }

    // {
    //     IngredientServiceTest ingredientServiceTest;
    //     status |= QTest::qExec(&ingredientServiceTest, argc, argv);
    // }

    // {
    //     RecipeFilterServiceTest recipeFilterServiceTest;
    //     status |= QTest::qExec(&recipeFilterServiceTest, argc, argv);
    // }

    // {
    //     AuthProcessorTest authProcessorTest;
    //     status |= QTest::qExec(&authProcessorTest, argc, argv);
    // }

    // {
    //     UserProcessorTest userProcessorTest;
    //     status |= QTest::qExec(&userProcessorTest, argc, argv);
    // }

    // {
    //     RecipeExplorerProcessorTest recipeExplorerProcessorTest;
    //     status |= QTest::qExec(&recipeExplorerProcessorTest, argc, argv);
    // }

    // {
    //     FavoriteProcessorTest favoriteProcessorTest;
    //     status |= QTest::qExec(&favoriteProcessorTest, argc, argv);
    // }

    // {
    //     ReviewProcessorTest reviewProcessorTest;
    //     status |= QTest::qExec(&reviewProcessorTest, argc, argv);
    // }

    // Интеграционные тесты

    {
        DbConnectionTest dbConnectionTest;
        status |= QTest::qExec(&dbConnectionTest, argc, argv);
    }

    {
        FridgeFusionIntegrationTest fridgeFusionIntegrationTest;
        status |= QTest::qExec(&fridgeFusionIntegrationTest, argc, argv);
    }


    return status;
}
