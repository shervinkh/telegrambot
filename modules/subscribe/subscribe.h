#ifndef SUBSCRIBE_H
#define SUBSCRIBE_H

#include "module.h"

class Subscribe : public Module
{
private:
    QString fSubscribe(qint64 gid, qint64 uid, const QDateTime &subscribed_on);
    QString fUnsubscribe(qint64 gid, qint64 uid);
    void sendNotification(qint64 gid, const QString &tag, const QString &text);
    void sendForward(qint64 gid, const QString &tag, qint64 msgId);
    QList<qint64> groupSubscribedUsers(qint64 gid);

protected:
    ModuleHelp help() const override;
    void registerModels() override;

public:
    Subscribe();
    void init() override;
    void onNewMessage(BInputMessage message) override;
    QVariant customCommand(const QString &command, const QList<QVariant> &args) override;
};

#endif // SUBSCRIBE_H
