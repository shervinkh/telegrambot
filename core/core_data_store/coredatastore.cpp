#include "coredatastore.h"
#include "redis.h"

#include <QTimeZone>

CoreDataStore::CoreDataStore(QObject *parent)
    : QObject(parent)
{
    mCoreRedis = new Redis("core", this);
    mMetaRedis = new Redis("meta", this);
}

Redis *CoreDataStore::redis(RedisInstance instance)
{
    switch (instance)
    {
        case CoreRedis:
            return mCoreRedis;
        case MetaRedis:
            return mMetaRedis;
    }

    return Q_NULLPTR;
}
