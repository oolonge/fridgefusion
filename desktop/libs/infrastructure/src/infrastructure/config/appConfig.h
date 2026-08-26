#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>
#include <QSettings>
#include <QStandardPaths>
#include <QJsonObject>

class AppConfig {
public:
    // Singleton pattern
    static AppConfig& getInstance();

    // Database connection settings
    QString getDbHost() const;
    int getDbPort() const;
    QString getDbName() const;
    QString getDbUsername() const;
    QString getDbPassword() const;

    // Set database connection settings (for testing/configuration)
    void setDbHost(const QString& host);
    void setDbPort(int port);
    void setDbName(const QString& name);
    void setDbUsername(const QString& username);
    void setDbPassword(const QString& password);

    // General application settings
    QString getAppName() const;
    QString getAppVersion() const;

    QString getLogDirectory() const;
    void setLogDirectory(const QString& path);

    // Методы для работы с JSON конфигурацией
    bool loadFromJsonFile(const QString& filePath = "config.json");
    bool saveToJsonFile(const QString& filePath = "config.json") const;
    QString getConfigFilePath() const;

private:
    AppConfig();
    ~AppConfig();

    // Disable copy constructor and assignment operator
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;

    void loadSettings();
    void saveSettings();

    // Новые методы для работы с JSON
    QJsonObject createDefaultConfig() const;
    void loadFromJson(const QJsonObject& json);
    QString findConfigFile() const;

    QSettings settings;

    // Default database configuration
    QString dbHost = "localhost";
    int dbPort = 5432;
    QString dbName = "postgres";
    QString dbUsername = "postgres";
    QString dbPassword = "";

    QString logDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";

    // Путь к файлу конфигурации
    QString configFilePath = "config.json";
};

#endif // APPCONFIG_H
