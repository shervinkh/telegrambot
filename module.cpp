#include "module.h"
#include "botinterface.h"
#include "bot.h"
#include "model.h"

Module::Module(const QString name, const qint64 version, const QDate &versionDate, QObject *parent)
    : QObject(parent), mName(name.toLower()), mVersion(version), mVersionDate(versionDate),
      mLoggingCategoryName(QByteArray("bot.modules.").append(mName)),
      mRedis(Q_NULLPTR), mConfig(Q_NULLPTR), mLoggingCategory(mLoggingCategoryName.data())
{
}

Module::~Module()
{

}

void Module::internalInit()
{
    registerModels();
    registerConfigs();
}

BotConfig *Module::config()
{
    if (!mConfig)
    {
        mConfig = new BotConfig(mBotInterface, QString("modules_%1").arg(mName));
        mConfig->addField("enabled_global", ConfigField::Boolean, true, true);
        mConfig->addField("enabled", ConfigField::Boolean, true);
    }

    return mConfig;
}

void Module::registerConfigs()
{
    auto conf = config();
    conf->registerConfig();
}

Model *Module::newModel(const QString &name, qint64 version, const QDate &versionDate)
{
    return interface()->newModel(QString("modules_%1").arg(mName), name, version, versionDate);
}

Model *Module::model(const QString &name)
{
    return interface()->model(QString("modules_%1").arg(mName), name);
}

Redis *Module::redis()
{
    if (!mRedis)
        mRedis = new Redis(QString("modules.%1").arg(mName), this);

    return mRedis;
}

QString Module::helpString() const
{
    auto moduleHelp = help();

    auto result = tr("Module: %1 -- Version: %2 (%3)").arg(mName).arg(mVersion)
            .arg(mVersionDate.toString(BotUtils::DATE_FORMAT));

    if (!moduleHelp.description().isEmpty())
        result += "\n" + moduleHelp.description();

    auto firstNote = true;
    foreach (auto note, moduleHelp.notes())
    {
        if (firstNote)
            result += "\n";

        result += tr("\nNote: %1").arg(note);

        firstNote = false;
    }

    foreach (auto usage, moduleHelp.usages())
    {
        result += tr("\n\nUsage: %1").arg(usage.usage());
        if (!usage.usageFormat().isEmpty())
            result += tr("\nFormat: %1").arg(usage.usageFormat());
        if (!usage.usageExample().isEmpty())
            result += tr("\nExample: %1").arg(usage.usageExample());
    }

    return result;
}
