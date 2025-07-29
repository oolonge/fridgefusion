#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>
#include <QSettings>
#include <QStandardPaths>

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

private:
    AppConfig();
    ~AppConfig();

    // Disable copy constructor and assignment operator
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;

    void loadSettings();
    void saveSettings();

    QSettings settings;

    // Default database configuration
    QString dbHost = "localhost";
    int dbPort = 5432;
    QString dbName = "postgres";
    QString dbUsername = "postgres";
    QString dbPassword = "";

    QString logDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
};

#endif // APPCONFIG_H
