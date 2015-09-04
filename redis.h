#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <QObject>
#include <QVariant>
#include <QLoggingCategory>

class QTextStream;

Q_DECLARE_LOGGING_CATEGORY(BOT_REDIS)

class Redis : public QObject
{
    Q_OBJECT
private:
    redisContext *mConnection;

    void reconnectRedis();
    void postExecute(redisReply *reply);
    QVariant processReply(redisReply *reply, bool first = true);

public:
    Redis(QObject *parent = Q_NULLPTR);

    //Primitive
    QVariant get(const QString &key);
    QVariant set(const QString &key, const QVariant &value, int ttl = -1, bool ifNotExists = false, bool ifExists = false);
    QVariant del(const QString &key);
    QVariant incr(const QString &key, int by=1);
    QVariant decr(const QString &key, int by=1);

    //Set
    QVariant sadd(const QString &key, const QVariant &value);
    QVariant smembers(const QString &key);

    //Hash
    QVariant hset(const QString &key, const QString &field, const QVariant &value);
    QVariant hget(const QString &key, const QString &field);
};

#endif // REDIS_H
