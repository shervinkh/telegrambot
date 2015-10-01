#include "botconfig.h"
#include "botinterface.h"

BotConfig::BotConfig()
{

}

BotConfig::BotConfig(BotInterface *botInterface, const QString &section, const QString &name,
                     qint64 version, const QDate &versionDate)
    : mBotInterface(botInterface), mSection("config_" + section), mName(name),
      mVersion(version), mVersionDate(versionDate)
{
    mModel = new Model(mBotInterface, mSection, mName, mVersion, mVersionDate);
    mModel->overrideDatabaseTable(fullName());
    mModel->addField("cid", ModelField::Integer);
    mModel->addField("name", ModelField::String);
    mModel->addField("value", ModelField::String);
    mBotInterface->registerModel(mModel);
}
