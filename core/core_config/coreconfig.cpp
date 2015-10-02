#include "coreconfig.h"
#include "botconfig.h"

CoreConfig::CoreConfig(BotInterface *botInterface, QObject *parent)
    : QObject(parent), mBotInterface(botInterface)
{

}

void CoreConfig::registerConfig(BotConfig *botConfig)
{
    auto assertWhere = QString("Registering new config \"%1\"").arg(botConfig->name()).toLocal8Bit();
    auto assertWhat = QByteArray("Config with the same name already exists");

    Q_ASSERT_X(!mRegisteredConfigs.contains(botConfig->name()), assertWhere.data(), assertWhat.data());

    mRegisteredConfigs[botConfig->name()] = botConfig;
}

BotConfig *CoreConfig::config(const QString &name)
{
    if (mRegisteredConfigs.contains(name))
        return mRegisteredConfigs[name];

    return Q_NULLPTR;
}
