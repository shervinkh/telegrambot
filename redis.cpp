#include "redis.h"
#include <QtCore>

redisContext *Redis::redisConnection = Q_NULLPTR;

Redis::Redis(const QString &name, QObject *parent)
    : QObject(parent), mName(name), mLoggingCategoryName(QByteArray("bot.redis.").append(mName)),
      mLoggingCategory(mLoggingCategoryName.data())
{
}

redisContext *Redis::redis()
{
    if (!redisConnection)
        reconnectRedis();

    return redisConnection;
}

void Redis::reconnectRedis()
{
    if (redisConnection != Q_NULLPTR)
        redisFree(redisConnection);

    redisConnection = redisConnectUnix("/tmp/redis.sock");
    if (redisConnection == Q_NULLPTR || redisConnection->err)
        qFatal("Couldn't connect to redis: %s", redisConnection->errstr);
}

void Redis::postExecute(redisReply *reply)
{
    freeReplyObject(reply);
    if (redisConnection->err)
        reconnectRedis();
}

QVariant Redis::processReply(redisReply *reply, bool first)
{
    QVariant result;

    if (reply == Q_NULLPTR)
        qCCritical(mLoggingCategory) << "Redis Error: " << redisConnection->errstr << endl << flush;
    else if (reply->type == REDIS_REPLY_STATUS)
        result = reply->str;
    else if (reply->type == REDIS_REPLY_INTEGER)
        result = reply->integer;
    else if (reply->type == REDIS_REPLY_STRING)
    {
        QByteArray data;
        data.resize(reply->len);
        qCopy(reply->str, reply->str + reply->len, data.data());
        result = data;
    }
    else if (reply->type == REDIS_REPLY_NIL)
        result = QVariant();
    else if (reply->type == REDIS_REPLY_ERROR)
        qCCritical(mLoggingCategory) << "Redis Error: " << reply->str << endl << flush;
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

QByteArray Redis::getActualKey(const QString &key)
{
    return QString("bot.%1.%2").arg(mName).arg(key).toLocal8Bit();
}

QVariant Redis::variadicCommand(const QString &command, const QString &key, const QList<QVariant> &values)
{
    QList<QByteArray> stringValues;
    stringValues.append(command.toLocal8Bit());
    stringValues.append(getActualKey(key));
    foreach (QVariant val, values)
        stringValues.append(val.toByteArray());

    const char **argv = new const char *[stringValues.size()];
    size_t *argvlen = new size_t[stringValues.size()];

    for (int i = 0; i < stringValues.size(); i++)
    {
        argv[i] = stringValues[i].data();
        argvlen[i] = stringValues[i].size();
    }

    redisReply *reply = (redisReply *) redisCommandArgv(redis(), stringValues.size(), argv, argvlen);
    delete [] argvlen;
    delete [] argv;
    return processReply(reply);
}

QVariant Redis::exists(const QString &key)
{
    return command("EXISTS", key);
}

QVariant Redis::get(const QString &key)
{
    return command("GET", key);
}

QVariant Redis::set(const QString &key, const QVariant &value, int ttl, bool ifNotExists, bool ifExists)
{
    QList<QVariant> args;

    args.append(value);

    if (ttl > 0)
    {
        args.append("EX");
        args.append(ttl);
    }

    if (ifNotExists && !ifExists)
        args.append("NX");
    else if (!ifNotExists && ifExists)
        args.append("XX");

    return variadicCommand("SET", key, args);
}

QVariant Redis::del(const QString &key)
{
    return command("DEL", key);
}

QVariant Redis::incr(const QString &key, int by)
{
    return command("INCRBY", key, by);
}

QVariant Redis::decr(const QString &key, int by)
{
    return command("DECRBY", key, by);
}

QVariant Redis::sadd(const QString &key, const QVariant &value)
{
    return command("SADD", key, value);
}

QVariant Redis::sadd(const QString &key, const QList<QVariant> &values)
{
    return variadicCommand("SADD", key, values);
}

QVariant Redis::scard(const QString &key)
{
    return command("SCARD", key);
}

QVariant Redis::smembers(const QString &key)
{
    return command("SMEMBERS", key);
}

QVariant Redis::hset(const QString &key, const QString &field, const QVariant &value)
{
    return command("HSET", key, field, value);
}

QVariant Redis::hexists(const QString &key, const QString &field)
{
    return command("HEXISTS", key, field);
}

QVariant Redis::hget(const QString &key, const QString &field)
{
    return command("HGET", key, field);
}

QVariant Redis::hdel(const QString &key, const QString &field)
{
    return command("HDEL", key, field);
}

QVariant Redis::getCachedValue(const QString &key, std::function<QVariant ()> calculateFunction)
{
    auto cacheKey = QString("cache.%1").arg(key);

    if (!exists(cacheKey).toBool())
    {
        auto result = calculateFunction();
        set(cacheKey, BotUtils::serialize(result));
    }

    return BotUtils::deserialize(get(cacheKey).toByteArray());
}

void Redis::invalidateCache(const QString &key)
{
    auto cacheKey = QString("cache.%1").arg(key);
    del(cacheKey);
}
