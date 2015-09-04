#include "redis.h"
#include <QtCore>

Q_LOGGING_CATEGORY(BOT_REDIS, "bot.redis")

Redis::Redis(QObject *parent)
    : QObject(parent), mConnection(Q_NULLPTR)
{
    reconnectRedis();
}

void Redis::reconnectRedis()
{
    if (mConnection != Q_NULLPTR)
        redisFree(mConnection);

    mConnection = redisConnectUnix("/tmp/redis.sock");
    if (mConnection == Q_NULLPTR || mConnection->err)
        qFatal("Couldn't connect to redis: %s", mConnection->errstr);
}

void Redis::postExecute(redisReply *reply)
{
    freeReplyObject(reply);
    if (mConnection->err)
        reconnectRedis();
}

QVariant Redis::processReply(redisReply *reply, bool first)
{
    QVariant result;

    if (reply == Q_NULLPTR)
        qCCritical(BOT_REDIS) << "Redis Error: " << mConnection->errstr << endl << flush;
    else if (reply->type == REDIS_REPLY_STATUS)
        result = reply->str;
    else if (reply->type == REDIS_REPLY_INTEGER)
        result = reply->integer;
    else if (reply->type == REDIS_REPLY_STRING)
        result = reply->str;
    else if (reply->type == REDIS_REPLY_NIL)
        result = QVariant();
    else if (reply->type == REDIS_REPLY_ERROR)
        qCCritical(BOT_REDIS) << "Redis Error: " << reply->str << endl << flush;
    else if (reply->type == REDIS_REPLY_ARRAY)
    {
        QList<QVariant> innerResult;
        for (unsigned int i = 0; i < reply->elements; i++)
            innerResult.append(processReply(reply->element[i], false));
        result = innerResult;
    }

    if (first)
        postExecute(reply);
    return result;
}

QVariant Redis::get(const QString &key)
{
    redisReply *reply = (redisReply *) redisCommand(mConnection, "GET %s", key.toStdString().c_str());
    return processReply(reply);
}

QVariant Redis::set(const QString &key, const QVariant &value, int ttl, bool ifNotExists, bool ifExists)
{
    QString ttlString = (ttl > 0) ? QString(" EX %1").arg(ttl) : "";
    QString existString = (ifNotExists && !ifExists) ? " NX" : ((!ifNotExists && ifExists) ? " XX" : "");
    redisReply *reply = (redisReply *) redisCommand(mConnection,
                                                    QString("SET %s %s%1%2").arg(ttlString).arg(existString).toStdString().c_str(),
                                                    key.toStdString().c_str(),
                                                    value.toString().toStdString().c_str());
    return processReply(reply);
}

QVariant Redis::del(const QString &key)
{
    redisReply *reply = (redisReply *) redisCommand(mConnection, "DEL %s", key.toStdString().c_str());
    return processReply(reply);
}

QVariant Redis::incr(const QString &key, int by)
{
    redisReply *reply = (redisReply *) redisCommand(mConnection, "INCRBY %s %lld", key.toStdString().c_str(), by);
    return processReply(reply);
}

QVariant Redis::decr(const QString &key, int by)
{
    redisReply *reply = (redisReply *) redisCommand(mConnection, "DECRBY %s %lld", key.toStdString().c_str(), by);
    return processReply(reply);
}

QVariant Redis::sadd(const QString &key, const QVariant &value)
{
    redisReply *reply = (redisReply *) redisCommand(mConnection, "SADD %s %s",
                                                    key.toStdString().c_str(), value.toString().toStdString().c_str());
    return processReply(reply);
}

QVariant Redis::smembers(const QString &key)
{
    redisReply *reply = (redisReply *) redisCommand(mConnection, "SMEMBERS %s", key.toStdString().c_str());
    return processReply(reply);
}

QVariant Redis::hset(const QString &key, const QString &field, const QVariant &value)
{
    redisReply *reply = (redisReply *) redisCommand(mConnection, "HSET %s %s %s", key.toStdString().c_str(),
                                                    field.toStdString().c_str(), value.toString().toStdString().c_str());
    return processReply(reply);
}

QVariant Redis::hget(const QString &key, const QString &field)
{
    redisReply *reply = (redisReply *) redisCommand(mConnection, "HGET %s %s", key.toStdString().c_str(),
                                                    field.toStdString().c_str());
    return processReply(reply);
}
