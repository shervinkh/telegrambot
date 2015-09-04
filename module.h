#ifndef MODULE_H
#define MODULE_H

#include "binputmessage.h"

#include <QObject>
#include <QLoggingCategory>

class Bot;

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
