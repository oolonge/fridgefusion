#ifndef FILELOGGER_H
#define FILELOGGER_H

#include "BaseLogger.h"
#include <QFile>
#include <QTextStream>

/**
 * @brief Реализация логера для записи в файл
 */
class FileLogger : public BaseLogger {
public:
    // Конструктор с указанием пути к файлу логов
    explicit FileLogger(const QString& filePath, LogLevel minLevel = LogLevel::INFO);
    ~FileLogger() override;

    // Реализация методов интерфейса
    void flush() override;
    bool isInitialized() const override;

protected:
    // Реализация метода записи в файл
    void writeLog(LogLevel level, const QString& formattedMessage) override;

private:
    // Инициализация логера с проверкой доступности файла
    bool initialize(const QString& filePath);

    QString filePath;
    QFile logFile;
    QTextStream stream;
    bool initialized;
};

#endif // FILELOGGER_H
