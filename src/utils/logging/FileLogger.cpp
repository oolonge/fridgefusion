#include "FileLogger.h"
#include <QDir>
#include <QFileInfo>

FileLogger::FileLogger(const QString& filePath, LogLevel minLevel)
    : BaseLogger(minLevel), filePath(filePath), initialized(false)
{
    initialized = initialize(filePath);
}

FileLogger::~FileLogger()
{
    if (logFile.isOpen()) {
        flush();
        logFile.close();
    }
}

bool FileLogger::initialize(const QString& filePath)
{
    // Проверяем существование директории
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();

    if (!dir.exists() && !dir.mkpath(".")) {
        // Не удалось создать директорию для логов
        return false;
    }

    // Открываем файл для записи (с дописыванием, если файл уже существует)
    logFile.setFileName(filePath);
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return false;
    }

    stream.setDevice(&logFile);
    return true;
}

void FileLogger::writeLog(LogLevel level, const QString& formattedMessage)
{
    if (!initialized || !logFile.isOpen())
        return;

    stream << formattedMessage << Qt::endl;

    // Автоматический сброс для сообщений уровня ERROR и FATAL
    if (level >= LogLevel::ERROR) {
        flush();
    }
}

void FileLogger::flush()
{
    if (initialized && logFile.isOpen()) {
        stream.flush();
        logFile.flush();
    }
}

bool FileLogger::isInitialized() const
{
    return initialized;
}
