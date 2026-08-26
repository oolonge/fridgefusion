#include "appConfig.h"

AppConfig& AppConfig::getInstance() {
    static AppConfig instance;
    return instance;
}

AppConfig::AppConfig()
    : settings("FridgeFusion", "FridgeFusion") {
    loadSettings();
}

AppConfig::~AppConfig() {
    saveSettings();
}

void AppConfig::loadSettings() {
    // Load database settings with values from your configuration
    dbHost = settings.value("Database/Host", "localhost").toString();
    dbPort = settings.value("Database/Port", 5432).toInt();
    dbName = settings.value("Database/Name", "postgres").toString();
    dbUsername = settings.value("Database/Username", "postgres").toString();
    dbPassword = settings.value("Database/Password", "").toString();
    logDirectory = settings.value("Logs/Directory",
                                  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs").toString();

}

void AppConfig::saveSettings() {
    // Save database settings
    settings.setValue("Database/Host", dbHost);
    settings.setValue("Database/Port", dbPort);
    settings.setValue("Database/Name", dbName);
    settings.setValue("Database/Username", dbUsername);
    settings.setValue("Database/Password", dbPassword);
    settings.setValue("Logs/Directory", logDirectory);

    settings.sync();
}

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

QString AppConfig::getLogDirectory() const
{
    return logDirectory;
}

void AppConfig::setLogDirectory(const QString& path)
{
    logDirectory = path;
    settings.setValue("Logs/Directory", logDirectory);
    settings.sync();
}
