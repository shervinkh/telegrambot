#include "coredatastore.h"
#include "installedmodulemodel.h"
#include "redis.h"
#include "module.h"

#include <QTimeZone>

DEFINE_MODEL(InstalledModuleModel)

CoreDataStore::CoreDataStore(BotInterface *botInterface, QObject *parent)
    : QObject(parent), mBotInterface(botInterface)
{
    ensureDatabase();

    mCoreRedis = new Redis("core", this);
    mMetaRedis = new Redis("meta", this);

    mBotInterface->registerModel("core", MODEL(InstalledModuleModel));
}

void CoreDataStore::ensureDatabase()
{
    mBotInterface->executeDatabaseQuery("CREATE TABLE IF NOT EXISTS bot_core_installed_modules ("
                               "    id bigserial PRIMARY KEY,"
                               "    name text NOT NULL,"
                               "    installed_version bigint NOT NULL,"
                               "    version_date timestamp with time zone,"
                               "    installed_date timestamp with time zone"
                               ")");

    mBotInterface->executeDatabaseQuery("CREATE UNIQUE INDEX unique_name_index "
                               "ON bot_core_installed_modules(name)");
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

void CoreDataStore::updateModuleInfo(Module *module)
{
    auto moduleInfos = MODEL(InstalledModuleModel)->objectSet().filter("name=?", module->name()).select();

    auto moduleInfo = moduleInfos.isEmpty() ? MODEL(InstalledModuleModel)->newInstance() : moduleInfos.first();

    if (moduleInfo->property("id").toLongLong() != -1 && moduleInfo->property("installed_version").toLongLong() == module->version())
        return;

    if (moduleInfo->property("id").toLongLong() == -1)
        moduleInfo->setProperty("name", module->name());

    moduleInfo->setProperty("installed_version", module->version());
    moduleInfo->setProperty("version_date", QDateTime(module->versionDate()));
    moduleInfo->setProperty("installed_date", QDateTime::currentDateTime());

    moduleInfo->save();
}
