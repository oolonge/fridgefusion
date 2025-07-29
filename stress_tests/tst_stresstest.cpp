#include <QtTest>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QThreadPool>
#include <QRunnable>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <QAtomicInt>
#include <QThread>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>

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

// Задача для выполнения в потоке
class StressTestTask : public QRunnable {
public:
    StressTestTask(int taskId, int scenarioType, QSemaphore* semaphore,
                   QMutex* resultMutex, QList<double>* results,
                   const QList<int>& existingRecipeIds);

    void run() override;

private:
    int taskId;
    int scenarioType;
    QSemaphore* semaphore;
    QMutex* resultMutex;
    QList<double>* results;
    QList<int> existingRecipeIds;

    // Методы для выполнения сценариев
    double executeUserRegistrationScenario();
    double executeRecipeSearchScenario();
    double executeFavoriteManagementScenario();
    double executeReviewManagementScenario();

    // Утилиты
    QString generateUniqueUsername();
    QString generateUniqueEmail();

    // Компоненты для работы с БД (создаются в каждом потоке)
    std::shared_ptr<DbContext> createDbContext();
    void setupComponents(std::shared_ptr<DbContext> dbContext,
                         std::shared_ptr<UserRepository>& userRepo,
                         std::shared_ptr<RecipeRepository>& recipeRepo,
                         std::shared_ptr<IngredientRepository>& ingredientRepo,
                         std::shared_ptr<FavoriteRepository>& favoriteRepo,
                         std::shared_ptr<ReviewRepository>& reviewRepo,
                         std::shared_ptr<AuthProcessor>& authProcessor,
                         std::shared_ptr<RecipeExplorerProcessor>& recipeProcessor,
                         std::shared_ptr<FavoriteProcessor>& favoriteProcessor,
                         std::shared_ptr<ReviewProcessor>& reviewProcessor);
};

class StressTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void runStressTest();
    void cleanupTestCase();

private:
    // Константы для тестирования
    static const int MIN_CONNECTIONS = 1;
    static const int MAX_CONNECTIONS = 100;
    static const int CONNECTION_STEP = 5;
    static const int SCENARIOS_PER_CONNECTION = 300;
    static const int MAX_THREAD_COUNT = 200; // Максимальное количество потоков

    // Данные для тестирования
    QList<int> existingRecipeIds;
    QThreadPool* threadPool;

    // Методы
    void saveResults(const QList<QPair<int, double>>& results);
    void cleanupDatabase();
};


StressTestTask::StressTestTask(int taskId, int scenarioType, QSemaphore* semaphore,
                               QMutex* resultMutex, QList<double>* results,
                               const QList<int>& existingRecipeIds)
    : taskId(taskId), scenarioType(scenarioType), semaphore(semaphore),
    resultMutex(resultMutex), results(results), existingRecipeIds(existingRecipeIds) {
    setAutoDelete(true);
}

void StressTestTask::run() {
    // Захватываем семафор (ограничиваем количество одновременно выполняющихся задач)
    semaphore->acquire();

    double responseTime = 0.0;

    try {
        switch (scenarioType) {
        case 0:
            responseTime = executeUserRegistrationScenario();
            break;
        case 1:
            responseTime = executeRecipeSearchScenario();
            break;
        case 2:
            responseTime = executeFavoriteManagementScenario();
            break;
        case 3:
            responseTime = executeReviewManagementScenario();
            break;
        }
    } catch (const std::exception& e) {
        qWarning() << QString("Ошибка в задаче %1 (сценарий %2): %3")
                          .arg(taskId).arg(scenarioType).arg(e.what());
    }

    // Записываем результат в потокобезопасном режиме
    {
        QMutexLocker locker(resultMutex);
        results->append(responseTime);
    }

    // Освобождаем семафор
    semaphore->release();
}

std::shared_ptr<DbContext> StressTestTask::createDbContext() {
    // Получаем ID потока и конвертируем в строку
    quintptr threadId = reinterpret_cast<quintptr>(QThread::currentThreadId());
    QString connectionName = QString("stress_thread_%1_%2")
                                 .arg(QString::number(threadId, 16))
                                 .arg(taskId);

    // Убеждаемся, что драйверы SQL доступны в текущем потоке
    if (!QSqlDatabase::drivers().contains("QPSQL")) {
        qWarning() << "PostgreSQL драйвер недоступен в потоке:" << connectionName;
        throw std::runtime_error("PostgreSQL драйвер недоступен");
    }

    // Создаем соединение с базой данных для текущего потока
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", connectionName);

    // Закрываем соединение если оно существует, чтобы избежать утечек
    if (db.isOpen()) {
        db.close();
    }

    auto dbContext = std::make_shared<DbContext>(connectionName);
    if (!dbContext->connect()) {
        QString error = dbContext->lastError();
        qWarning() << "Не удалось подключиться к БД в потоке" << connectionName << ":" << error;

        // Удаляем соединение при ошибке
        QSqlDatabase::removeDatabase(connectionName);
        throw std::runtime_error("Не удалось подключиться к базе данных");
    }
    return dbContext;
}

void StressTestTask::setupComponents(std::shared_ptr<DbContext> dbContext,
                                     std::shared_ptr<UserRepository>& userRepo,
                                     std::shared_ptr<RecipeRepository>& recipeRepo,
                                     std::shared_ptr<IngredientRepository>& ingredientRepo,
                                     std::shared_ptr<FavoriteRepository>& favoriteRepo,
                                     std::shared_ptr<ReviewRepository>& reviewRepo,
                                     std::shared_ptr<AuthProcessor>& authProcessor,
                                     std::shared_ptr<RecipeExplorerProcessor>& recipeProcessor,
                                     std::shared_ptr<FavoriteProcessor>& favoriteProcessor,
                                     std::shared_ptr<ReviewProcessor>& reviewProcessor) {

    // Создаем общие сервисы (потокобезопасные)
    auto passwordHasher = std::make_shared<PasswordHasher>();
    auto recipeFilterService = std::make_shared<RecipeFilterService>();

    // Создаем репозитории
    userRepo = std::make_shared<UserRepository>(dbContext);
    recipeRepo = std::make_shared<RecipeRepository>(dbContext);
    ingredientRepo = std::make_shared<IngredientRepository>(dbContext);
    favoriteRepo = std::make_shared<FavoriteRepository>(dbContext.get());
    reviewRepo = std::make_shared<ReviewRepository>(dbContext.get());

    // Создаем процессоры
    authProcessor = std::make_shared<AuthProcessor>(userRepo, passwordHasher);
    recipeProcessor = std::make_shared<RecipeExplorerProcessor>(recipeRepo, ingredientRepo, recipeFilterService);
    favoriteProcessor = std::make_shared<FavoriteProcessor>(favoriteRepo.get());
    reviewProcessor = std::make_shared<ReviewProcessor>(reviewRepo.get());
}

double StressTestTask::executeUserRegistrationScenario() {
    QElapsedTimer timer;
    timer.start();

    auto dbContext = createDbContext();
    std::shared_ptr<UserRepository> userRepo;
    std::shared_ptr<RecipeRepository> recipeRepo;
    std::shared_ptr<IngredientRepository> ingredientRepo;
    std::shared_ptr<FavoriteRepository> favoriteRepo;
    std::shared_ptr<ReviewRepository> reviewRepo;
    std::shared_ptr<AuthProcessor> authProcessor;
    std::shared_ptr<RecipeExplorerProcessor> recipeProcessor;
    std::shared_ptr<FavoriteProcessor> favoriteProcessor;
    std::shared_ptr<ReviewProcessor> reviewProcessor;

    setupComponents(dbContext, userRepo, recipeRepo, ingredientRepo, favoriteRepo, reviewRepo,
                    authProcessor, recipeProcessor, favoriteProcessor, reviewProcessor);

    // Генерируем уникальные данные
    QString username = generateUniqueUsername();
    QString email = generateUniqueEmail();
    QString password = "stress_test_password_123";

    int userId = -1;

    try {
        // Регистрация
        AuthResult regResult = authProcessor->registerUser(username, email, password);
        if (!regResult.isSuccess()) {
            qWarning() << "Ошибка регистрации в потоке:" << regResult.getErrorMessage();
            return timer.elapsed();
        }

        userId = regResult.getUser()->getId();

        // Выход
        authProcessor->logout();

        // Вход
        AuthResult loginResult = authProcessor->login(email, password);
        if (!loginResult.isSuccess()) {
            qWarning() << "Ошибка входа в потоке:" << loginResult.getErrorMessage();
        }

        // Выход
        authProcessor->logout();

    } catch (const std::exception& e) {
        qWarning() << "Исключение в сценарии регистрации (поток):" << e.what();
    }

    // Очистка: удаляем пользователя
    if (userId > 0) {
        try {
            userRepo->deleteUser(userId);
        } catch (const std::exception& e) {
            qWarning() << "Ошибка удаления пользователя:" << e.what();
        }
    }

    // Отключаемся от БД и удаляем соединение
    if (dbContext->isConnected()) {
        dbContext->disconnect();
    }

    // Получаем имя соединения для удаления
    quintptr threadId = reinterpret_cast<quintptr>(QThread::currentThreadId());
    QString connectionName = QString("stress_thread_%1_%2")
                                 .arg(QString::number(threadId, 16))
                                 .arg(taskId);

    // Удаляем соединение из пула Qt
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }

    return timer.elapsed();
}

double StressTestTask::executeRecipeSearchScenario() {
    QElapsedTimer timer;
    timer.start();

    auto dbContext = createDbContext();
    std::shared_ptr<UserRepository> userRepo;
    std::shared_ptr<RecipeRepository> recipeRepo;
    std::shared_ptr<IngredientRepository> ingredientRepo;
    std::shared_ptr<FavoriteRepository> favoriteRepo;
    std::shared_ptr<ReviewRepository> reviewRepo;
    std::shared_ptr<AuthProcessor> authProcessor;
    std::shared_ptr<RecipeExplorerProcessor> recipeProcessor;
    std::shared_ptr<FavoriteProcessor> favoriteProcessor;
    std::shared_ptr<ReviewProcessor> reviewProcessor;

    setupComponents(dbContext, userRepo, recipeRepo, ingredientRepo, favoriteRepo, reviewRepo,
                    authProcessor, recipeProcessor, favoriteProcessor, reviewProcessor);

    try {
        // Получаем ингредиенты
        QList<Ingredient> ingredients = ingredientRepo->getAllIngredients();
        if (!ingredients.isEmpty()) {
            // Выбираем случайные ингредиенты
            QList<int> searchIngredients;
            int count = qMin(3, ingredients.size());
            for (int i = 0; i < count; i++) {
                int randomIndex = QRandomGenerator::global()->bounded(ingredients.size());
                searchIngredients.append(ingredients[randomIndex].getIngredientId());
            }

            // Поиск рецептов
            QList<Recipe> foundRecipes = recipeProcessor->findRecipesWithIngredients(searchIngredients);

            // Получение деталей первого рецепта
            if (!foundRecipes.isEmpty()) {
                RecipeDetailDTO detail = recipeProcessor->getRecipeDetail(foundRecipes.first().getRecipeId());
                Q_UNUSED(detail)
            }
        }

        // Поиск по категории
        QList<Category> categories = recipeProcessor->getAllCategories();
        if (!categories.isEmpty()) {
            int randomCategoryId = categories[QRandomGenerator::global()->bounded(categories.size())].getCategoryId();
            QList<Recipe> categoryRecipes = recipeProcessor->findRecipesByCategory(randomCategoryId);
            Q_UNUSED(categoryRecipes)
        }

    } catch (const std::exception& e) {
        qWarning() << "Исключение в сценарии поиска (поток):" << e.what();
    }

    // Отключаемся от БД и удаляем соединение
    if (dbContext->isConnected()) {
        dbContext->disconnect();
    }

    // Получаем имя соединения для удаления
    quintptr threadId = reinterpret_cast<quintptr>(QThread::currentThreadId());
    QString connectionName = QString("stress_thread_%1_%2")
                                 .arg(QString::number(threadId, 16))
                                 .arg(taskId);

    // Удаляем соединение из пула Qt
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }

    return timer.elapsed();
}

double StressTestTask::executeFavoriteManagementScenario() {
    QElapsedTimer timer;
    timer.start();

    auto dbContext = createDbContext();
    std::shared_ptr<UserRepository> userRepo;
    std::shared_ptr<RecipeRepository> recipeRepo;
    std::shared_ptr<IngredientRepository> ingredientRepo;
    std::shared_ptr<FavoriteRepository> favoriteRepo;
    std::shared_ptr<ReviewRepository> reviewRepo;
    std::shared_ptr<AuthProcessor> authProcessor;
    std::shared_ptr<RecipeExplorerProcessor> recipeProcessor;
    std::shared_ptr<FavoriteProcessor> favoriteProcessor;
    std::shared_ptr<ReviewProcessor> reviewProcessor;

    setupComponents(dbContext, userRepo, recipeRepo, ingredientRepo, favoriteRepo, reviewRepo,
                    authProcessor, recipeProcessor, favoriteProcessor, reviewProcessor);

    int userId = -1;

    try {
        // Создаем временного пользователя
        QString username = generateUniqueUsername();
        QString email = generateUniqueEmail();
        AuthResult regResult = authProcessor->registerUser(username, email, "temp_password");

        if (regResult.isSuccess() && !existingRecipeIds.isEmpty()) {
            userId = regResult.getUser()->getId();

            // Выбираем случайный рецепт
            int recipeId = existingRecipeIds[QRandomGenerator::global()->bounded(existingRecipeIds.size())];

            // Добавляем в избранное
            favoriteProcessor->addToFavorites(userId, recipeId);

            // Проверяем статус
            bool isFavorite = favoriteProcessor->isRecipeFavorite(userId, recipeId);
            Q_UNUSED(isFavorite)

            // Получаем список избранного
            QList<Favorite> favorites = favoriteProcessor->getUserFavorites(userId);
            Q_UNUSED(favorites)

            // Удаляем из избранного
            favoriteProcessor->removeFromFavorites(userId, recipeId);
        }

    } catch (const std::exception& e) {
        qWarning() << "Исключение в сценарии избранного (поток):" << e.what();
    }

    // Очистка: удаляем пользователя
    if (userId > 0) {
        try {
            userRepo->deleteUser(userId);
        } catch (const std::exception& e) {
            qWarning() << "Ошибка удаления пользователя в избранном:" << e.what();
        }
    }

    // Отключаемся от БД и удаляем соединение
    if (dbContext->isConnected()) {
        dbContext->disconnect();
    }

    // Получаем имя соединения для удаления
    quintptr threadId = reinterpret_cast<quintptr>(QThread::currentThreadId());
    QString connectionName = QString("stress_thread_%1_%2")
                                 .arg(QString::number(threadId, 16))
                                 .arg(taskId);

    // Удаляем соединение из пула Qt
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }

    return timer.elapsed();
}

double StressTestTask::executeReviewManagementScenario() {
    QElapsedTimer timer;
    timer.start();

    auto dbContext = createDbContext();
    std::shared_ptr<UserRepository> userRepo;
    std::shared_ptr<RecipeRepository> recipeRepo;
    std::shared_ptr<IngredientRepository> ingredientRepo;
    std::shared_ptr<FavoriteRepository> favoriteRepo;
    std::shared_ptr<ReviewRepository> reviewRepo;
    std::shared_ptr<AuthProcessor> authProcessor;
    std::shared_ptr<RecipeExplorerProcessor> recipeProcessor;
    std::shared_ptr<FavoriteProcessor> favoriteProcessor;
    std::shared_ptr<ReviewProcessor> reviewProcessor;

    setupComponents(dbContext, userRepo, recipeRepo, ingredientRepo, favoriteRepo, reviewRepo,
                    authProcessor, recipeProcessor, favoriteProcessor, reviewProcessor);

    int userId = -1;

    try {
        // Создаем временного пользователя
        QString username = generateUniqueUsername();
        QString email = generateUniqueEmail();
        AuthResult regResult = authProcessor->registerUser(username, email, "temp_password");

        if (regResult.isSuccess() && !existingRecipeIds.isEmpty()) {
            userId = regResult.getUser()->getId();

            // Выбираем случайный рецепт
            int recipeId = existingRecipeIds[QRandomGenerator::global()->bounded(existingRecipeIds.size())];

            // Создаем отзыв
            int rating = QRandomGenerator::global()->bounded(5) + 1;
            QString comment = QString("Стресс-тест отзыв %1").arg(taskId);

            bool addResult = reviewProcessor->addReview(userId, recipeId, rating, comment);
            if (addResult) {
                // Получаем отзывы
                QList<Review> userReviews = reviewProcessor->getUserReviews(userId);
                QList<Review> recipeReviews = reviewProcessor->getRecipeReviews(recipeId);
                double avgRating = reviewProcessor->getRecipeAverageRating(recipeId);
                Q_UNUSED(recipeReviews)
                Q_UNUSED(avgRating)

                // Удаляем наш отзыв
                for (const Review& review : userReviews) {
                    if (review.getUserId() == userId && review.getRecipeId() == recipeId) {
                        reviewProcessor->deleteReview(review.getId());
                        break;
                    }
                }
            }
        }

    } catch (const std::exception& e) {
        qWarning() << "Исключение в сценарии отзывов (поток):" << e.what();
    }

    // Очистка: удаляем пользователя
    if (userId > 0) {
        try {
            userRepo->deleteUser(userId);
        } catch (const std::exception& e) {
            qWarning() << "Ошибка удаления пользователя в отзывах:" << e.what();
        }
    }

    // Отключаемся от БД и удаляем соединение
    if (dbContext->isConnected()) {
        dbContext->disconnect();
    }

    // Получаем имя соединения для удаления
    quintptr threadId = reinterpret_cast<quintptr>(QThread::currentThreadId());
    QString connectionName = QString("stress_thread_%1_%2")
                                 .arg(QString::number(threadId, 16))
                                 .arg(taskId);

    // Удаляем соединение из пула Qt
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }

    return timer.elapsed();
}

QString StressTestTask::generateUniqueUsername() {
    quintptr threadId = reinterpret_cast<quintptr>(QThread::currentThreadId());
    return QString("stress_user_%1_%2_%3")
        .arg(QString::number(threadId, 16))
        .arg(QDateTime::currentMSecsSinceEpoch())
        .arg(taskId);
}

QString StressTestTask::generateUniqueEmail() {
    quintptr threadId = reinterpret_cast<quintptr>(QThread::currentThreadId());
    return QString("stress_test_%1_%2_%3@example.com")
        .arg(QString::number(threadId, 16))
        .arg(QDateTime::currentMSecsSinceEpoch())
        .arg(taskId);
}

// === Реализация StressTest ===

void StressTest::initTestCase() {
    qDebug() << "=== Инициализация многопоточного стресс-тестирования ===";

    // Проверяем доступность PostgreSQL драйвера
    if (!QSqlDatabase::drivers().contains("QPSQL")) {
        QFAIL("PostgreSQL драйвер (QPSQL) недоступен. Убедитесь, что он установлен.");
    }

    qDebug() << "Доступные SQL драйверы:" << QSqlDatabase::drivers();

    // Настраиваем пул потоков
    threadPool = QThreadPool::globalInstance();
    threadPool->setMaxThreadCount(MAX_THREAD_COUNT);
    qDebug() << "Максимальное количество потоков:" << threadPool->maxThreadCount();
    qDebug() << "Количество идеальных потоков:" << QThread::idealThreadCount();

    // Получаем существующие рецепты
    auto tempDbContext = std::make_shared<DbContext>("temp_init_connection");
    QVERIFY(tempDbContext->connect());

    auto tempRecipeRepo = std::make_shared<RecipeRepository>(tempDbContext);
    QList<Recipe> existingRecipes = tempRecipeRepo->getAllRecipes(20, 0);

    for (const Recipe& recipe : existingRecipes) {
        existingRecipeIds.append(recipe.getRecipeId());
    }

    tempDbContext->disconnect();

    // Удаляем временное соединение
    if (QSqlDatabase::contains("temp_init_connection")) {
        QSqlDatabase::removeDatabase("temp_init_connection");
    }

    QVERIFY(!existingRecipeIds.isEmpty());

    qDebug() << "Найдено существующих рецептов:" << existingRecipeIds.size();
    qDebug() << "Готов к многопоточному тестированию";
}

void StressTest::runStressTest() {
    qDebug() << "=== Начало многопоточного стресс-тестирования ===";

    QList<QPair<int, double>> results;

    for (int connectionCount = MIN_CONNECTIONS; connectionCount <= MAX_CONNECTIONS; connectionCount += CONNECTION_STEP) {
        qDebug() << QString("Тестирование с %1 одновременными подключениями...").arg(connectionCount);

        // Семафор для ограничения количества одновременно выполняющихся задач
        QSemaphore semaphore(connectionCount);

        // Мьютекс и список для сбора результатов
        QMutex resultMutex;
        QList<double> responseTimeResults;

        // Время начала тестирования для данного количества подключений
        QElapsedTimer totalTimer;
        totalTimer.start();

        // Создаем и запускаем задачи
        int totalTasks = SCENARIOS_PER_CONNECTION * 4; // 4 типа сценариев
        Q_UNUSED(totalTasks)

        for (int scenario = 0; scenario < SCENARIOS_PER_CONNECTION; scenario++) {
            for (int scenarioType = 0; scenarioType < 4; scenarioType++) {
                int taskId = scenario * 4 + scenarioType;

                auto task = new StressTestTask(taskId, scenarioType, &semaphore,
                                               &resultMutex, &responseTimeResults,
                                               existingRecipeIds);

                // Запускаем задачу в пуле потоков
                threadPool->start(task);
            }
        }

        // Ждем завершения всех задач
        threadPool->waitForDone();

        qint64 totalElapsed = totalTimer.elapsed();

        // Вычисляем среднее время ответа
        double avgResponseTime = 0.0;
        if (!responseTimeResults.isEmpty()) {
            for (double time : responseTimeResults) {
                avgResponseTime += time;
            }
            avgResponseTime /= responseTimeResults.size();
        }

        results.append(qMakePair(connectionCount, avgResponseTime));

        qDebug() << QString("Подключений: %1 | Среднее время: %2 мс | Общее время: %3 мс | Задач выполнено: %4")
                        .arg(connectionCount)
                        .arg(avgResponseTime, 0, 'f', 2)
                        .arg(totalElapsed)
                        .arg(responseTimeResults.size());

        // Небольшая пауза между тестами разной нагрузки
        QThread::msleep(1000);
    }

    // Сохраняем результаты
    saveResults(results);

    qDebug() << "=== Многопоточное стресс-тестирование завершено ===";
}

void StressTest::saveResults(const QList<QPair<int, double>>& results) {
    QString fileName = "multithreaded_stress_test_results.csv";
    QFile file(fileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "users;avg_response_time\n";

        for (const auto& result : results) {
            out << result.first << ";" << QString::number(result.second, 'f', 2) << "\n";
        }

        file.close();
        qDebug() << "Результаты многопоточного тестирования сохранены в файл:" << fileName;
    } else {
        qWarning() << "Не удалось сохранить результаты в файл:" << fileName;
    }
}

void StressTest::cleanupDatabase() {
    qDebug() << "Выполнение финальной очистки базы данных...";

    try {
        auto cleanupDbContext = std::make_shared<DbContext>("cleanup_connection");
        if (cleanupDbContext->connect()) {
            // Удаляем всех пользователей, созданных стресс-тестами
            cleanupDbContext->executeQuery(
                "DELETE FROM users WHERE username LIKE 'stress_user_%' OR email LIKE 'stress_test_%@example.com'"
                );

            // Удаляем потерянные отзывы от стресс-тестов
            cleanupDbContext->executeQuery(
                "DELETE FROM reviews WHERE comment LIKE 'Стресс-тест отзыв %' OR comment LIKE 'Тестовый отзыв от стресс-теста %'"
                );

            cleanupDbContext->disconnect();
            qDebug() << "Финальная очистка базы данных завершена";
        }
    } catch (const std::exception& e) {
        qWarning() << "Ошибка при финальной очистке:" << e.what();
    }
}

void StressTest::cleanupTestCase() {
    qDebug() << "Финальная очистка стресс-тестов...";

    // Ждем завершения всех задач в пуле потоков
    if (threadPool) {
        threadPool->waitForDone(30000); // Максимум 30 секунд ожидания
    }

    // Очищаем базу данных от тестовых данных
    cleanupDatabase();

    qDebug() << "Очистка стресс-тестов завершена";
}

#include "tst_stresstest.moc"
