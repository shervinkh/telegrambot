#ifndef MODULE_H
#define MODULE_H

#include "binputmessage.h"

#include <QObject>
#include <QLoggingCategory>

class Bot;

#define DECLARE_BOT_MODULE(name) \
    public: \
        static name *instance() { \
            if (!mInstance) \
                mInstance = new name(); \
            return mInstance; \
        } \
    private: \
        name(); \
        static name *mInstance;

#define DEFINE_BOT_MODULE(name) \
    name *name::mInstance = Q_NULLPTR;

#define BOT_MODULE(name) name::instance()

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

    Bot *mBot;

    const QByteArray mLoggingCategoryName;

protected:
    QLoggingCategory mLoggingCategory;

    void sendMessage(qint64 id, bool chat, const QString &message, qint64 replyTo);

public:
    explicit Module(const QString name, const qint64 version, QObject *parent = Q_NULLPTR);
    virtual ~Module() = 0;

    void internalInit();
    void setBot(Bot *bot) { mBot = bot; }

    QString name() { return mName; }
    qint64 version() { return mVersion; }

    virtual void init() {}
    virtual void onNewMessage(BInputMessage message) { Q_UNUSED(message); }

public slots:

};

#endif // MODULE_H
