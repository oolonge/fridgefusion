#ifndef BASELOGGER_H
#define BASELOGGER_H

#include "ILogger.h"
#include <QMutex>

/**
 * @brief Базовая реализация логера с общей функциональностью
 */
class BaseLogger : public ILogger {
public:
    explicit BaseLogger(LogLevel minLevel = LogLevel::INFO);
    virtual ~BaseLogger() = default;

    // Реализация методов интерфейса ILogger
    void debug(const QString& message) override;
    void info(const QString& message) override;
    void warn(const QString& message) override;
    void error(const QString& message) override;
    void fatal(const QString& message) override;
    void log(LogLevel level, const QString& message, const QString& context = QString()) override;
    void setMinLogLevel(LogLevel level) override;
    LogLevel getMinLogLevel() const override;

    // В базовом классе flush и isInitialized объявляем чисто виртуальными
    virtual void flush() override = 0;
    virtual bool isInitialized() const override = 0;

protected:
    // Абстрактный метод для вывода сообщения, реализуемый в наследниках
    virtual void writeLog(LogLevel level, const QString& formattedMessage) = 0;

    // Форматирование сообщения с временной меткой и уровнем
    QString formatMessage(LogLevel level, const QString& message, const QString& context = QString());

    // Преобразование уровня логирования в строку
    static QString logLevelToString(LogLevel level);

    // Текущий уровень логирования
    LogLevel minLogLevel;

    // Мьютекс для потокобезопасного логирования
    QMutex mutex;
};

#endif // BASELOGGER_H
