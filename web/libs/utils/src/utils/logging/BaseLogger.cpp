#include "BaseLogger.h"
#include <QDateTime>

BaseLogger::BaseLogger(LogLevel minLevel)
    : minLogLevel(minLevel)
{
}

void BaseLogger::debug(const QString& message)
{
    log(LogLevel::DEBUG, message);
}

void BaseLogger::info(const QString& message)
{
    log(LogLevel::INFO, message);
}

void BaseLogger::warn(const QString& message)
{
    log(LogLevel::WARN, message);
}

void BaseLogger::error(const QString& message)
{
    log(LogLevel::ERROR, message);
}

void BaseLogger::fatal(const QString& message)
{
    log(LogLevel::FATAL, message);
}

void BaseLogger::log(LogLevel level, const QString& message, const QString& context)
{
    if (level < minLogLevel)
        return;

    QMutexLocker locker(&mutex);
    QString formattedMessage = formatMessage(level, message, context);
    writeLog(level, formattedMessage);
}

void BaseLogger::setMinLogLevel(LogLevel level)
{
    minLogLevel = level;
}

LogLevel BaseLogger::getMinLogLevel() const
{
    return minLogLevel;
}

QString BaseLogger::formatMessage(LogLevel level, const QString& message, const QString& context)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = logLevelToString(level);

    QString formattedMessage;
    if (context.isEmpty()) {
        formattedMessage = QString("[%1] [%2] %3")
                               .arg(timestamp)
                               .arg(levelStr)
                               .arg(message);
    } else {
        formattedMessage = QString("[%1] [%2] [%3] %4")
                               .arg(timestamp)
                               .arg(levelStr)
                               .arg(context)
                               .arg(message);
    }

    return formattedMessage;
}

QString BaseLogger::logLevelToString(LogLevel level)
{
    switch (level) {
    case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::INFO:  return "INFO ";
    case LogLevel::WARN:  return "WARN ";
    case LogLevel::ERROR: return "ERROR";
    case LogLevel::FATAL: return "FATAL";
    default:              return "UNKNOWN";
    }
}
