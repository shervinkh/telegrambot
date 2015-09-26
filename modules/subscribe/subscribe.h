#ifndef SUBSCRIBE_H
#define SUBSCRIBE_H

#include "module.h"

class Subscribe : public Module
{
    DECLARE_MODULE(Subscribe)
private:
    QString fSubscribe(qint64 gid, qint64 uid, const QDateTime &subscribed_on);
    QString fUnsubscribe(qint64 gid, qint64 uid);
    void sendNotification(qint64 gid, const QString &tag, const QString &text);

protected:
    ModuleHelp help() const override;
    void ensureDatabase() override;
    void registerModels() override;

public:
    void init() override;
    void onNewMessage(BInputMessage message) override;
    void customCommand(const QString &command, const QList<QVariant> &args) override;
};

#endif // SUBSCRIBE_H
