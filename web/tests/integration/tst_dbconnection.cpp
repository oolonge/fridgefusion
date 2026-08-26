#include <QtTest>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QCoreApplication>
#include "infrastructure/persistence/database/dbContext.h"
#include "infrastructure/config/appConfig.h"

class DbConnectionTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testConnection();
    void cleanupTestCase();

private:
    DbContext* dbContext;
};

void DbConnectionTest::initTestCase() {
    // Вывод отладочной информации о доступных драйверах
    qDebug() << "Доступные драйверы SQL:";
    for (const QString& driver : QSqlDatabase::drivers()) {
        qDebug() << "  " << driver;
    }

    // Проверяем наличие PostgreSQL среди драйверов
    if (!QSqlDatabase::drivers().contains("QPSQL")) {
        qWarning() << "Драйвер PostgreSQL (QPSQL) не найден. "
                   << "Продолжаем тестирование DbContext, но подключение может не работать.";
    }

    qDebug() << "Создание экземпляра DbContext...";

    // Создаем экземпляр DbContext
    dbContext = new DbContext("integration_test");

    // Проверка, что экземпляр создан
    QVERIFY2(dbContext != nullptr, "DbContext не был создан");

    qDebug() << "DbContext успешно создан.";

    // Проверяем конфигурацию базы данных
    AppConfig& config = AppConfig::getInstance();

    qDebug() << "Конфигурация базы данных:";
    qDebug() << "  Хост: " << config.getDbHost();
    qDebug() << "  Порт: " << config.getDbPort();
    qDebug() << "  База: " << config.getDbName();
    qDebug() << "  Пользователь: " << config.getDbUsername();
}

void DbConnectionTest::testConnection() {
    qDebug() << "Попытка подключения к базе данных...";

    // Проверяем, есть ли драйвер перед попыткой подключения
    if (!QSqlDatabase::drivers().contains("QPSQL")) {
        QSKIP("Пропуск теста - драйвер PostgreSQL не доступен");
    }

    // Проверка возможности подключения к базе данных
    bool connected = dbContext->connect();

    if (!connected) {
        qDebug() << "Ошибка подключения: " << dbContext->lastError();
    }

    // Проверка результата подключения с выводом подробной информации об ошибке
    QVERIFY2(connected, qPrintable(QString("Не удалось подключиться к базе данных: %1").arg(dbContext->lastError())));

    qDebug() << "Подключение успешно установлено.";

    // Проверка, что статус подключения корректен
    QVERIFY(dbContext->isConnected());

    qDebug() << "Проверка статуса подключения успешна.";

    // Выполняем тестовый запрос
    qDebug() << "Выполнение тестового запроса...";

    QSqlQuery query = dbContext->executeQuery("SELECT 1 as test");

    if (query.lastError().isValid()) {
        qDebug() << "Ошибка запроса: " << query.lastError().text();
    }

    // Проверка выполнения запроса без ошибок
    QVERIFY2(!query.lastError().isValid(), qPrintable(query.lastError().text()));

    // Проверка наличия результата
    QVERIFY(query.next());

    // Проверка значения результата
    QCOMPARE(query.value(0).toInt(), 1);

    qDebug() << "Тестовый запрос выполнен успешно.";
}

void DbConnectionTest::cleanupTestCase() {
    qDebug() << "Отключение от базы данных...";

    // Отключаемся от базы данных, если подключены
    if (dbContext && dbContext->isConnected()) {
        dbContext->disconnect();
    }

    // Проверка, что отключение произошло успешно
    if (dbContext) {
        if (dbContext->isConnected()) {
            qWarning() << "Не удалось отключиться от базы данных";
        } else {
            qDebug() << "Отключение успешно выполнено.";
        }
    }

    // Освобождаем память
    delete dbContext;
    dbContext = nullptr;

    qDebug() << "Тест завершен.";
}

#include "tst_dbconnection.moc"
