#include "LoggerService.h"
#include "FileLogger.h"
#include "ConsoleLogger.h"
#include <QStandardPaths>
#include <QDir>

LoggerService& LoggerService::getInstance()
{
    static LoggerService instance;
    return instance;
}

LoggerService::LoggerService()
{
    // По умолчанию создаем консольный логер
    std::shared_ptr<ILogger> defaultLogger = createConsoleLogger();
    registerLogger("default", defaultLogger);
}

std::shared_ptr<ILogger> LoggerService::getLogger(const QString& loggerName)
{
    if (loggers.contains(loggerName)) {
        return loggers[loggerName];
    }

    // Если запрошенный логер не найден, возвращаем логер по умолчанию
    if (loggers.contains("default")) {
        return loggers["default"];
    }

    // Если и логер по умолчанию не найден, создаем консольный логер
    std::shared_ptr<ILogger> defaultLogger = createConsoleLogger();
    registerLogger("default", defaultLogger);
    return defaultLogger;
}

void LoggerService::registerLogger(const QString& loggerName, std::shared_ptr<ILogger> logger)
{
    loggers[loggerName] = logger;
}

void LoggerService::removeLogger(const QString& loggerName)
{
    loggers.remove(loggerName);
}

void LoggerService::setGlobalLogLevel(LogLevel level)
{
    for (auto& logger : loggers) {
        logger->setMinLogLevel(level);
    }
}

std::shared_ptr<ILogger> LoggerService::createFileLogger(const QString& filePath, LogLevel level)
{
    return std::make_shared<FileLogger>(filePath, level);
}

std::shared_ptr<ILogger> LoggerService::createConsoleLogger(LogLevel level)
{
    return std::make_shared<ConsoleLogger>(level);
}
