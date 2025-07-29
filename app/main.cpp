#include "src/presentation/forms/mainwindow.h"
#include "utils/logging/LoggerService.h"
#include "infrastructure/config/appConfig.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AppConfig& config = AppConfig::getInstance();
    LoggerService& logService = LoggerService::getInstance();

    std::shared_ptr<ILogger> consoleLogger = logService.createConsoleLogger(LogLevel::DEBUG);
    logService.registerLogger("console", consoleLogger);

    qDebug() << "Директория логов:" << AppConfig::getInstance().getLogDirectory();

    QString logFilePath = config.getLogDirectory() + "/fridgefusion.log";
    std::shared_ptr<ILogger> fileLogger = logService.createFileLogger(logFilePath, LogLevel::INFO);
    logService.registerLogger("file", fileLogger);

    std::shared_ptr<ILogger> logger = logService.getLogger("file");
    logger->info("Приложение FridgeFusion запущено");

    MainWindow w;
    w.show();
    return a.exec();
}
