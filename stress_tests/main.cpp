#include <QtTest>
#include <QCoreApplication>
#include <QDebug>
#include <QSqlDatabase>

#include "tst_stresstest.cpp"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Путь к плагинам SQL
    QCoreApplication::addLibraryPath("/opt/homebrew/Cellar/qt-postgresql/6.9.0/lib");

    // Проверка доступных SQL драйверов
    qDebug() << "Доступные SQL драйверы:";
    qDebug() << QSqlDatabase::drivers();

    int status = 0;

    {
        StressTest stressTest;
        status = QTest::qExec(&stressTest, argc, argv);
    }

    return status;
}
