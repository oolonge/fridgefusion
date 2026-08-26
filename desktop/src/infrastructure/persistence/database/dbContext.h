#ifndef DBCONTEXT_H
#define DBCONTEXT_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVariant>
#include <QList>
#include <QVector>
#include <QDebug>
#include <QSqlRecord>
#include <functional>

class DbContext {
public:
    explicit DbContext(const QString& connectionName = "default");
    ~DbContext();

    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const;

    // Transaction management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Query execution
    bool executeSql(const QString& sql);
    QSqlQuery executeQuery(const QString& sql);
    QSqlQuery executeQuery(const QString& sql, const QVariantMap& params);

    // Insert, update, delete operations
    bool insert(const QString& table, const QVariantMap& data);
    bool update(const QString& table, const QVariantMap& data, const QString& whereClause);
    bool remove(const QString& table, const QString& whereClause);

    // Query building helpers
    QSqlQuery select(const QString& table, const QStringList& columns = QStringList(),
                     const QString& whereClause = QString(), const QString& orderBy = QString(),
                     int limit = -1, int offset = -1);

    // Get last inserted ID
    QVariant lastInsertId() const;

    // Get last error
    QString lastError() const;

    // Get database instance directly (for complex operations)
    QSqlDatabase& getDatabase();

    // Execute a function in a transaction, with automatic commit/rollback
    template<typename Func>
    bool transactional(Func func) {
        bool success = false;
        if (beginTransaction()) {
            try {
                success = func();
                if (success) {
                    success = commitTransaction();
                } else {
                    rollbackTransaction();
                }
            } catch (...) {
                rollbackTransaction();
                throw;
            }
        }
        return success;
    }

private:
    QSqlDatabase db;
    QString connectionName;
    bool connected;
};

#endif // DBCONTEXT_H
