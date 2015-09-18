#ifndef REDIS_H
#define REDIS_H

#include "botutils.h"

#include <functional>
#include <hiredis/hiredis.h>
#include <QObject>
#include <QVariant>
#include <QLoggingCategory>

class QTextStream;

class Redis : public QObject
{
    Q_OBJECT
private:
    static redisContext *redisConnection;
    static redisContext *redis();

    static void reconnectRedis();
    static void postExecute(redisReply *reply);

    QString mName;

    const QByteArray mLoggingCategoryName;
    QLoggingCategory mLoggingCategory;

    QVariant processReply(redisReply *reply, bool first = true);
    QByteArray getActualKey(const QString &key);
    QVariant variadicCommand(const QString &command, const QString &key, const QList<QVariant> &values);

    template<typename... Types>
    QVariant command(const QString &command, const QString &key, Types... args)
    {
        QList<QVariant> argsList = BotUtils::convertArgsToList(args...);
        return variadicCommand(command, key, argsList);
    }

public:
    Redis(const QString &name, QObject *parent = Q_NULLPTR);

    //Primitive
    QVariant exists(const QString &key);
    QVariant get(const QString &key);
    QVariant set(const QString &key, const QVariant &value, int ttl = -1, bool ifNotExists = false, bool ifExists = false);
    QVariant del(const QString &key);
    QVariant incr(const QString &key, int by=1);
    QVariant decr(const QString &key, int by=1);

    //Set
    QVariant sadd(const QString &key, const QVariant &value);
    QVariant sadd(const QString &key, const QList<QVariant> &values);
    QVariant scard(const QString &key);
    QVariant smembers(const QString &key);

    //Hash
    QVariant hset(const QString &key, const QString &field, const QVariant &value);
    QVariant hexists(const QString &key, const QString &field);
    QVariant hget(const QString &key, const QString &field);
    QVariant hdel(const QString &key, const QString &field);

    //Cache
    QVariant getCachedValue(const QString &key, std::function<QVariant()> calculateFunction);
    void invalidateCache(const QString &key);
};

#endif // REDIS_H
