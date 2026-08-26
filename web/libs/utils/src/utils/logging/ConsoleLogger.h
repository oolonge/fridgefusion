#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include "BaseLogger.h"
#include <QTextStream>

/**
 * @brief Реализация логера для вывода в консоль
 */
class ConsoleLogger : public BaseLogger {
public:
    explicit ConsoleLogger(LogLevel minLevel = LogLevel::INFO);
    ~ConsoleLogger() override = default;

    // Реализация методов интерфейса
    void flush() override;
    bool isInitialized() const override;

protected:
    // Реализация метода вывода в консоль
    void writeLog(LogLevel level, const QString& formattedMessage) override;

private:
    QTextStream outStream;
    QTextStream errStream;
};

#endif // CONSOLELOGGER_H
