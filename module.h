#ifndef MODULE_H
#define MODULE_H

#include "binputmessage.h"
#include "botutils.h"
#include "bot.h"
#include "redis.h"

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
    const QByteArray mLoggingCategoryName;

    Redis *mRedis;
    Bot *mBot;

protected:
    QLoggingCategory mLoggingCategory;

    virtual void ensureDatabase() {}
    void registerModel(QObject *model);

    Redis *redis() { return mRedis; }

public:
    explicit Module(const QString name, const qint64 version, QObject *parent = Q_NULLPTR);
    virtual ~Module() = 0;

    void internalInit();
    void setBot(Bot *bot) { mBot = bot;}

    QString name() { return mName; }
    qint64 version() { return mVersion; }

    virtual void init() {}
    virtual void onNewMessage(BInputMessage message) { Q_UNUSED(message); }

    //Model Interface
    BotInterface *interface() { return mBot->interface(); }
    QString getModelDatabaseTable(QObject *object);
    int saveModelObject(QObject *object);
    int deleteModelObject(QObject *object);

    //Cache Interface
    QString getCacheKey(const QString &key);

public slots:

};

#endif // MODULE_H
