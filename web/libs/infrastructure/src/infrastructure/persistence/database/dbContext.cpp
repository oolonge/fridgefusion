#include "dbContext.h"
#include "infrastructure/config/appConfig.h"

DbContext::DbContext(const QString& connectionName)
    : connectionName(connectionName), connected(false) {

    if (QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::addDatabase("QPSQL", connectionName);
    }
}

DbContext::~DbContext() {
    if (connected) {
        disconnect();
    }
}

bool DbContext::connect() {
    if (connected) {
        return true;
    }

    AppConfig& config = AppConfig::getInstance();

    db.setHostName(config.getDbHost());
    db.setPort(config.getDbPort());
    db.setDatabaseName(config.getDbName());
    db.setUserName(config.getDbUsername());
    db.setPassword(config.getDbPassword());

    connected = db.open();

    if (!connected) {
        qDebug() << "Failed to connect to database:" << db.lastError().text();
    } else {
        qDebug() << "Connected to database" << config.getDbName() << "on" << config.getDbHost();
    }

    return connected;
}

void DbContext::disconnect() {
    if (connected) {
        db.close();
        connected = false;
        qDebug() << "Disconnected from database";
    }
}

bool DbContext::isConnected() const {
    return connected && db.isOpen();
}

bool DbContext::beginTransaction() {
    if (!isConnected() && !connect()) {
        return false;
    }

    return db.transaction();
}

bool DbContext::commitTransaction() {
    return db.commit();
}

bool DbContext::rollbackTransaction() {
    return db.rollback();
}

bool DbContext::executeSql(const QString& sql) {
    if (!isConnected() && !connect()) {
        return false;
    }

    QSqlQuery query(db);
    bool success = query.exec(sql);

    if (!success) {
        qDebug() << "SQL Error:" << query.lastError().text();
        qDebug() << "Query:" << sql;
    }

    return success;
}

QSqlQuery DbContext::executeQuery(const QString& sql) {
    QSqlQuery query(db);

    if (!isConnected() && !connect()) {
        return query;
    }

    if (!query.exec(sql)) {
        qDebug() << "SQL Error:" << query.lastError().text();
        qDebug() << "Query:" << sql;
    }

    return query;
}

QSqlQuery DbContext::executeQuery(const QString& sql, const QVariantMap& params) {
    QSqlQuery query(db);

    if (!isConnected() && !connect()) {
        return query;
    }

    query.prepare(sql);

    for (auto it = params.begin(); it != params.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text();
        qDebug() << "Query:" << sql;
        qDebug() << "Params:" << params;
    }

    return query;
}

bool DbContext::insert(const QString& table, const QVariantMap& data) {
    if (!isConnected() && !connect()) {
        return false;
    }

    QStringList fields;
    QStringList placeholders;
    QVariantMap bindValues;

    for (auto it = data.begin(); it != data.end(); ++it) {
        fields << it.key();
        placeholders << (":" + it.key());
        bindValues.insert(it.key(), it.value());
    }

    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(table)
                      .arg(fields.join(", "))
                      .arg(placeholders.join(", "));

    QSqlQuery query = executeQuery(sql, bindValues);
    return !query.lastError().isValid();
}

bool DbContext::update(const QString& table, const QVariantMap& data, const QString& whereClause) {
    if (!isConnected() && !connect()) {
        return false;
    }

    QStringList setParts;
    QVariantMap bindValues;

    for (auto it = data.begin(); it != data.end(); ++it) {
        setParts << (it.key() + " = :" + it.key());
        bindValues.insert(it.key(), it.value());
    }

    QString sql = QString("UPDATE %1 SET %2").arg(table).arg(setParts.join(", "));

    if (!whereClause.isEmpty()) {
        sql += " WHERE " + whereClause;
    }

    QSqlQuery query = executeQuery(sql, bindValues);
    return !query.lastError().isValid();
}

bool DbContext::remove(const QString& table, const QString& whereClause) {
    if (!isConnected() && !connect()) {
        return false;
    }

    QString sql = QString("DELETE FROM %1").arg(table);

    if (!whereClause.isEmpty()) {
        sql += " WHERE " + whereClause;
    }

    return executeSql(sql);
}

QSqlQuery DbContext::select(const QString& table, const QStringList& columns, const QString& whereClause,
                            const QString& orderBy, int limit, int offset) {

    QString columnsStr = columns.isEmpty() ? "*" : columns.join(", ");

    QString sql = QString("SELECT %1 FROM %2").arg(columnsStr).arg(table);

    if (!whereClause.isEmpty()) {
        sql += " WHERE " + whereClause;
    }

    if (!orderBy.isEmpty()) {
        sql += " ORDER BY " + orderBy;
    }

    if (limit > 0) {
        sql += QString(" LIMIT %1").arg(limit);
    }

    if (offset > 0) {
        sql += QString(" OFFSET %1").arg(offset);
    }

    return executeQuery(sql);
}

QVariant DbContext::lastInsertId() const {
    QSqlQuery query(db);
    query.exec("SELECT lastval()");

    if (query.next()) {
        return query.value(0);
    }

    return QVariant();
}

QString DbContext::lastError() const {
    return db.lastError().text();
}

QSqlDatabase& DbContext::getDatabase() {
    return db;
}
