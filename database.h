#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QLoggingCategory>
#include <QSqlDatabase>

class QMutex;
class QSqlQuery;
class QTextStream;

Q_DECLARE_LOGGING_CATEGORY(BOT_DATABASE)

class Database : public QObject
{
    Q_OBJECT
private:
    const QString mTablePrefix;

    QSqlDatabase mDatabase;
    QMutex *mMutex;

public:
    explicit Database(QObject *parent = 0);
    ~Database();

    void execute(QSqlQuery &query);
};

#endif // DATABASE_H
