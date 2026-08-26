#include "appConfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
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

    // Загружаем настройки API
    QJsonObject api = json["api"].toObject();

    // Server settings
    QJsonObject server = api["server"].toObject();
    apiHost = server["host"].toString("0.0.0.0");
    apiPort = server["port"].toInt(8080);
    apiThreads = server["threads"].toInt(4);
    apiCompressionEnabled = server["enable_compression"].toBool(true);

    // Override API port from environment variable if set
    QString envPort = qgetenv("API_PORT");
    if (!envPort.isEmpty()) {
        bool ok;
        int port = envPort.toInt(&ok);
        if (ok && port > 0 && port < 65536) {
            apiPort = port;
        }
    }

    // JWT settings
    QJsonObject jwt = api["jwt"].toObject();
    jwtSecret = jwt["secret"].toString("change-this-secret-key-in-production");
    jwtExpirationHours = jwt["expiration_hours"].toInt(24);
    jwtIssuer = jwt["issuer"].toString("FridgeFusion API v2.0");

    // CORS settings
    QJsonObject cors = api["cors"].toObject();
    corsEnabled = cors["enabled"].toBool(true);

    QJsonArray originsArray = cors["allowed_origins"].toArray();
    corsAllowedOrigins.clear();
    for (const QJsonValue& origin : originsArray) {
        corsAllowedOrigins.append(origin.toString());
    }
    if (corsAllowedOrigins.isEmpty()) {
        corsAllowedOrigins = {"*"};
    }

    QJsonArray methodsArray = cors["allowed_methods"].toArray();
    corsAllowedMethods.clear();
    for (const QJsonValue& method : methodsArray) {
        corsAllowedMethods.append(method.toString());
    }
    if (corsAllowedMethods.isEmpty()) {
        corsAllowedMethods = {"GET", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"};
    }

    QJsonArray headersArray = cors["allowed_headers"].toArray();
    corsAllowedHeaders.clear();
    for (const QJsonValue& header : headersArray) {
        corsAllowedHeaders.append(header.toString());
    }
    if (corsAllowedHeaders.isEmpty()) {
        corsAllowedHeaders = {"Content-Type", "Authorization", "X-Requested-With"};
    }
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

// API Server settings
QString AppConfig::getApiHost() const {
    return apiHost;
}

int AppConfig::getApiPort() const {
    return apiPort;
}

int AppConfig::getApiThreads() const {
    return apiThreads;
}

bool AppConfig::getApiCompressionEnabled() const {
    return apiCompressionEnabled;
}

// JWT settings
QString AppConfig::getJwtSecret() const {
    return jwtSecret;
}

int AppConfig::getJwtExpirationHours() const {
    return jwtExpirationHours;
}

QString AppConfig::getJwtIssuer() const {
    return jwtIssuer;
}

// CORS settings
bool AppConfig::getCorsEnabled() const {
    return corsEnabled;
}

QStringList AppConfig::getCorsAllowedOrigins() const {
    return corsAllowedOrigins;
}

QStringList AppConfig::getCorsAllowedMethods() const {
    return corsAllowedMethods;
}

QStringList AppConfig::getCorsAllowedHeaders() const {
    return corsAllowedHeaders;
}
