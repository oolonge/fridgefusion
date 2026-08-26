#include "appConfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

AppConfig& AppConfig::getInstance() {
    static AppConfig instance;
    return instance;
}

AppConfig::AppConfig()
    : settings("FridgeFusion", "FridgeFusion") {

    // Определяем путь к конфигурационному файлу
    QString configPath = findConfigFile();
    configFilePath = configPath;

    // Пытаемся загрузить из JSON файла
    if (!loadFromJsonFile(configFilePath)) {
        qDebug() << "Не удалось загрузить JSON конфигурацию из" << configFilePath;
        qDebug() << "Используем настройки по умолчанию";
        // Если JSON не загрузился, пробуем старый способ через QSettings
        loadSettings();
        // Создаём JSON файл с текущими настройками в корне проекта
        saveToJsonFile(configFilePath);
    }
}

AppConfig::~AppConfig() {
    // Сохраняем в JSON при выходе
    saveToJsonFile(configFilePath);
}

void AppConfig::loadSettings() {
    // Загружаем настройки из QSettings (старый способ)
    dbHost = settings.value("Database/Host", "localhost").toString();
    dbPort = settings.value("Database/Port", 5432).toInt();
    dbName = settings.value("Database/Name", "postgres").toString();
    dbUsername = settings.value("Database/Username", "postgres").toString();
    dbPassword = settings.value("Database/Password", "").toString();
    logDirectory = settings.value("Logs/Directory",
                                  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs").toString();
}

void AppConfig::saveSettings() {
    // Сохраняем в QSettings (для совместимости)
    settings.setValue("Database/Host", dbHost);
    settings.setValue("Database/Port", dbPort);
    settings.setValue("Database/Name", dbName);
    settings.setValue("Database/Username", dbUsername);
    settings.setValue("Database/Password", dbPassword);
    settings.setValue("Logs/Directory", logDirectory);
    settings.sync();
}

bool AppConfig::loadFromJsonFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Не удалось открыть файл конфигурации:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "Ошибка парсинга JSON конфигурации:" << parseError.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qDebug() << "JSON конфигурация должна быть объектом";
        return false;
    }

    loadFromJson(doc.object());
    qDebug() << "Конфигурация успешно загружена из:" << filePath;
    return true;
}

bool AppConfig::saveToJsonFile(const QString& filePath) const {
    QJsonObject config;

    // Database settings
    QJsonObject database;
    database["host"] = dbHost;
    database["port"] = dbPort;
    database["name"] = dbName;
    database["username"] = dbUsername;
    database["password"] = dbPassword;
    config["database"] = database;

    // Logging settings
    QJsonObject logging;
    logging["directory"] = logDirectory;
    config["logging"] = logging;

    QJsonDocument doc(config);

    // Создаём директорию если её нет
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Не удалось открыть файл для записи:" << filePath;
        return false;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Конфигурация сохранена в:" << filePath;
    return true;
}

void AppConfig::loadFromJson(const QJsonObject& json) {
    // Загружаем настройки базы данных
    QJsonObject database = json["database"].toObject();
    dbHost = database["host"].toString("localhost");
    dbPort = database["port"].toInt(5432);
    dbName = database["name"].toString("postgres");
    dbUsername = database["username"].toString("postgres");
    dbPassword = database["password"].toString("");

    // Загружаем настройки логирования
    QJsonObject logging = json["logging"].toObject();
    QString defaultLogDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    logDirectory = logging["directory"].toString(defaultLogDir);
}

QJsonObject AppConfig::createDefaultConfig() const {
    QJsonObject config;

    QJsonObject database;
    database["host"] = "localhost";
    database["port"] = 5432;
    database["name"] = "postgres";
    database["username"] = "postgres";
    database["password"] = "";
    config["database"] = database;

    QJsonObject logging;
    logging["directory"] = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    config["logging"] = logging;

    return config;
}

QString AppConfig::findConfigFile() const {
    // Список потенциальных путей для поиска config.json
    QStringList searchPaths;

    // 1. Переменная окружения (если задана)
    QString envPath = qgetenv("FRIDGEFUSION_CONFIG");
    if (!envPath.isEmpty()) {
        searchPaths << envPath;
    }

    // 2. Текущая рабочая директория
    searchPaths << QDir::currentPath() + "/config.json";

    // 3. Поднимаемся вверх по директориям от исполняемого файла (для macOS .app бандлов)
    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);

    // На macOS поднимаемся из .app/Contents/MacOS обратно к корню проекта
    for (int i = 0; i < 6; ++i) { // максимум 6 уровней вверх
        QString configPath = dir.absolutePath() + "/config.json";
        searchPaths << configPath;

        if (!dir.cdUp()) {
            break;
        }
    }

    // 4. Рядом с исполняемым файлом (последний приоритет)
    searchPaths << appDir + "/config.json";

    // Ищем первый существующий файл
    for (const QString& path : searchPaths) {
        if (QFile::exists(path)) {
            qDebug() << "Найден конфигурационный файл:" << path;
            return path;
        }
    }

    // Если ничего не найдено, используем текущую рабочую директорию
    QString defaultPath = QDir::currentPath() + "/config.json";
    qDebug() << "Конфигурационный файл не найден, будет создан:" << defaultPath;
    return defaultPath;
}

QString AppConfig::getConfigFilePath() const {
    return configFilePath;
}

// Остальные методы остаются без изменений
QString AppConfig::getDbHost() const {
    return dbHost;
}

int AppConfig::getDbPort() const {
    return dbPort;
}

QString AppConfig::getDbName() const {
    return dbName;
}

QString AppConfig::getDbUsername() const {
    return dbUsername;
}

QString AppConfig::getDbPassword() const {
    return dbPassword;
}

void AppConfig::setDbHost(const QString& host) {
    dbHost = host;
}

void AppConfig::setDbPort(int port) {
    dbPort = port;
}

void AppConfig::setDbName(const QString& name) {
    dbName = name;
}

void AppConfig::setDbUsername(const QString& username) {
    dbUsername = username;
}

void AppConfig::setDbPassword(const QString& password) {
    dbPassword = password;
}

QString AppConfig::getAppName() const {
    return "FridgeFusion";
}

QString AppConfig::getAppVersion() const {
    return "1.0.0";
}

QString AppConfig::getLogDirectory() const {
    return logDirectory;
}

void AppConfig::setLogDirectory(const QString& path) {
    logDirectory = path;
    settings.setValue("Logs/Directory", logDirectory);
    settings.sync();
}
