#include "ConsoleLogger.h"
#include <QDebug>

ConsoleLogger::ConsoleLogger(LogLevel minLevel)
    : BaseLogger(minLevel), outStream(stdout), errStream(stderr)
{
}

void ConsoleLogger::writeLog(LogLevel level, const QString& formattedMessage)
{
    // Ошибки и критические ошибки выводим в stderr, остальное в stdout
    if (level >= LogLevel::ERROR) {
        errStream << formattedMessage << Qt::endl;
    } else {
        outStream << formattedMessage << Qt::endl;

        // Для отладки также выводим в QDebug
        if (level == LogLevel::DEBUG) {
            qDebug().noquote() << formattedMessage;
        }
    }
}

void ConsoleLogger::flush()
{
    outStream.flush();
    errStream.flush();
}

bool ConsoleLogger::isInitialized() const
{
    return true; // Консольный логер всегда инициализирован
}
