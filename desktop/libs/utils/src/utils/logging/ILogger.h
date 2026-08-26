#ifndef ILOGGER_H
#define ILOGGER_H

#include <QString>
#include <QDateTime>

enum class LogLevel {
    DEBUG,  // Подробная отладочная информация
    INFO,   // Информационные сообщения
    WARN,   // Предупреждения
    ERROR,  // Ошибки
    FATAL   // Критические ошибки
};

class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void debug(const QString& message) = 0;
    virtual void info(const QString& message) = 0;
    virtual void warn(const QString& message) = 0;
    virtual void error(const QString& message) = 0;
    virtual void fatal(const QString& message) = 0;

    // Логирование с дополнительным контекстом
    virtual void log(LogLevel level, const QString& message, const QString& context = QString()) = 0;

    // Настройка минимального уровня логирования
    virtual void setMinLogLevel(LogLevel level) = 0;
    virtual LogLevel getMinLogLevel() const = 0;

    virtual void flush() = 0; // Принудительная запись буферизованных логов
    virtual bool isInitialized() const = 0; // Проверка инициализации логера
};

#endif // ILOGGER_H
