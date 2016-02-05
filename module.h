#ifndef MODULE_H
#define MODULE_H

#include "binputmessage.h"
#include "botutils.h"
#include "bot.h"
#include "redis.h"
#include "botconfig.h"
#include "core/help/modulehelp.h"

#include <QObject>
#include <QVariant>
#include <QSqlQuery>
#include <QLoggingCategory>

#define MODULE(name) name::instance()

#define logDebug() qCDebug(mLoggingCategory)
#define logInfo() qCInfo(mLoggingCategory)
#define logWarning() qCWarning(mLoggingCategory)
#define logCritical() qCCritical(mLoggingCategory)

class Model;

class Module : public QObject
{
    Q_OBJECT
private:
    const QString mName;
    const qint64 mVersion;
    const QDate mVersionDate;
    const QByteArray mLoggingCategoryName;

    Redis *mRedis;
    BotInterface *mBotInterface;

    BotConfig *mConfig;

    QList<QString> mSupportingCommands;

protected:
    QLoggingCategory mLoggingCategory;

    virtual void registerModels() {}
    virtual void registerConfigs();
    virtual ModuleHelp help() const { return ModuleHelp(); }
    void registerCommand(const QString &command) { mSupportingCommands.append(command); }
    BotConfig *config();

    Model *newModel(const QString &name, qint64 version, const QDate &versionDate);
    Model *model(const QString &name);

    Redis *redis();

public:
    explicit Module(const QString name, const qint64 version, const QDate &versionDate, QObject *parent = Q_NULLPTR);
    virtual ~Module() = 0;

    void internalInit();
    void setBotInterface(BotInterface *botInterface) { mBotInterface = botInterface;}

    QString name() const { return mName; }
    qint64 version() const { return mVersion; }
    QDate versionDate() const { return mVersionDate; }
    QString helpString() const;
    QList<QString> supportingCommands() const { return mSupportingCommands; }

    virtual void init() {}

    virtual void onNewMessage(BInputMessage message) { Q_UNUSED(message); }

    virtual QVariant customCommand(const QString &command, const QList<QVariant> &args) { Q_UNUSED(command); Q_UNUSED(args); return QVariant(); }

    //Model Interface
    BotInterface *interface() { return mBotInterface; }

public slots:

};

#endif // MODULE_H
