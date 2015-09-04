#include "database.h"
#include <QtSql>
#include <QMutex>
#include <QMutexLocker>

Q_LOGGING_CATEGORY(BOT_DATABASE, "bot.database")

Database::Database(QObject *parent)
    : QObject(parent), mTablePrefix("telegbot")
{
    mDatabase = QSqlDatabase::addDatabase("QPSQL");
    mDatabase.setDatabaseName("telegram_bot");
    mDatabase.setUserName("postgres");
    mDatabase.setPassword("12345");
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
}
