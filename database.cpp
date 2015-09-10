#include "database.h"
#include <QtSql>
#include <QMutex>
#include <QMutexLocker>

Q_LOGGING_CATEGORY(BOT_DATABASE, "bot.database")

Database::Database(QObject *parent)
    : QObject(parent)
{
    mDatabase = QSqlDatabase::addDatabase("QPSQL");
    mDatabase.setDatabaseName("telegrambot");
    mDatabase.setUserName("telegrambot");
    if (!mDatabase.open())
        qFatal("Couldn't connect to the database: %s", mDatabase.lastError().text().toStdString().c_str());

    mMutex = new QMutex;
}

Database::~Database()
{
    delete mMutex;
}

void Database::execute(QSqlQuery &query)
{
    QMutexLocker mutexLocker(mMutex);
    Q_UNUSED(mutexLocker);

    if (!query.exec())
        qCCritical(BOT_DATABASE) << "Sql Error: " << query.executedQuery() << endl
                                 << query.lastError().text() << endl << flush;

    qCDebug(BOT_DATABASE) << "Executed Query: " << query.lastQuery() << query.lastInsertId() << query.numRowsAffected();
}

QSqlQuery Database::execute(const QString &query)
{
    QMutexLocker mutexLocker(mMutex);
    Q_UNUSED(mutexLocker);

    QSqlQuery sqlQuery(mDatabase);
    if (!sqlQuery.exec(query))
        qCCritical(BOT_DATABASE) << "Sql Error: " << query << endl
                                 << sqlQuery.lastError().text() << endl << flush;

    qCDebug(BOT_DATABASE) << "Executed Query: " << sqlQuery.lastQuery() << sqlQuery.lastInsertId() << sqlQuery.numRowsAffected();

    return sqlQuery;
}
