#ifndef MODULE_H
#define MODULE_H

#include "binputmessage.h"
#include "botutils.h"
#include "bot.h"
#include "redis.h"
#include "help/modulehelp.h"

#include <QObject>
#include <QVariant>
#include <QSqlQuery>
#include <QLoggingCategory>

#define DECLARE_MODULE(name) \
public: \
    static name *instance() { \
        if (!mInstance) \
            mInstance = new name(); \
        return mInstance; \
    } \
private: \
    name(); \
    static name *mInstance;

#define DEFINE_MODULE(name) \
    name *name::mInstance = Q_NULLPTR;

#define MODULE(name) name::instance()

#define logDebug() qCDebug(mLoggingCategory)
#define logInfo() qCInfo(mLoggingCategory)
#define logWarning() qCWarning(mLoggingCategory)
#define logCritical() qCCritical(mLoggingCategory)

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

    QList<QString> mSupportingCommands;

protected:
    QLoggingCategory mLoggingCategory;

    virtual void ensureDatabase() {}
    virtual ModuleHelp help() const { return ModuleHelp(); }
    void registerCommand(const QString &command) { mSupportingCommands.append(command); }
    void registerModel(QObject *model);

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

    virtual void customCommand(const QString &command, const QList<QVariant> &args) { Q_UNUSED(command); Q_UNUSED(args); }

    //Model Interface
    BotInterface *interface() { return mBotInterface; }

public slots:

};

#endif // MODULE_H
