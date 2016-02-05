#ifndef COREDATASTORE_H
#define COREDATASTORE_H

#include <QObject>

class BotInterface;
class Redis;

class CoreDataStore : public QObject
{
    Q_OBJECT
private:
    Redis *mCoreRedis;
    Redis *mMetaRedis;

public:
    enum RedisInstance {CoreRedis, MetaRedis};

    explicit CoreDataStore(QObject *parent = 0);
    Redis *redis(RedisInstance instance);
};

#endif // COREDATASTORE_H
