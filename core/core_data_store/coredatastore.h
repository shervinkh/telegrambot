#ifndef COREDATASTORE_H
#define COREDATASTORE_H

#include <QObject>

class BotInterface;
class Module;
class Redis;

class CoreDataStore : public QObject
{
    Q_OBJECT
private:
    BotInterface *mBotInterface;

    Redis *mCoreRedis;
    Redis *mMetaRedis;

    void ensureDatabase();

public:
    enum RedisInstance {CoreRedis, MetaRedis};

    explicit CoreDataStore(BotInterface *botInterface, QObject *parent = 0);
    Redis *redis(RedisInstance instance);

    void updateModuleInfo(Module *module);

};

#endif // COREDATASTORE_H
