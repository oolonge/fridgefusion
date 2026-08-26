#ifndef LOGGERSERVICE_H
#define LOGGERSERVICE_H

#include "ILogger.h"
#include <QMap>
#include <QString>
#include <memory>

/**
 * @brief Сервис для централизованного управления логированием
 */
class LoggerService {
public:
    // Получение экземпляра по шаблону Singleton
    static LoggerService& getInstance();

    // Получение логера по имени (создаёт, если отсутствует)
    std::shared_ptr<ILogger> getLogger(const QString& loggerName);

    // Регистрация нового логера
    void registerLogger(const QString& loggerName, std::shared_ptr<ILogger> logger);

    // Удаление логера
    void removeLogger(const QString& loggerName);

    // Настройка минимального уровня логирования для всех логеров
    void setGlobalLogLevel(LogLevel level);

    // Методы создания стандартных логеров
    std::shared_ptr<ILogger> createFileLogger(const QString& filePath, LogLevel level = LogLevel::INFO);
    std::shared_ptr<ILogger> createConsoleLogger(LogLevel level = LogLevel::INFO);

private:
    // Конструктор приватный для реализации Singleton
    LoggerService();

    // Запрет копирования и перемещения
    LoggerService(const LoggerService&) = delete;
    LoggerService& operator=(const LoggerService&) = delete;
    LoggerService(LoggerService&&) = delete;
    LoggerService& operator=(LoggerService&&) = delete;

    // Карта логеров по именам
    QMap<QString, std::shared_ptr<ILogger>> loggers;
};

#endif // LOGGERSERVICE_H
