#include "module.h"
#include "botinterface.h"
#include "bot.h"

Module::Module(const QString name, const qint64 version, const QDate &versionDate, QObject *parent)
    : QObject(parent), mName(name.toLower()), mVersion(version), mVersionDate(versionDate),
      mLoggingCategoryName(QByteArray("bot.modules.").append(mName)),
      mRedis(Q_NULLPTR), mLoggingCategory(mLoggingCategoryName.data())
{

}

Module::~Module()
{

}

void Module::internalInit()
{
    ensureDatabase();
}

void Module::registerModel(QObject *model)
{
    interface()->registerModel(QString("modules_%1").arg(mName), model);
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
