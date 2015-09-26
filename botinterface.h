#ifndef BOTINTERFACE_H
#define BOTINTERFACE_H

#include <QObject>
#include <QSqlQuery>

#include "bot.h"
#include "groupmetadata.h"
#include "model.h"
#include "modelobject.h"

class BotInterface : public QObject
{
    Q_OBJECT
private:
    Bot *mBot;

    InputPeer getPeer(qint64 id, bool chat);

public:
    explicit BotInterface(Bot *bot, QObject *parent = 0);

    QString aboutText();
    bool debug();

    Module *getModule(const QString &name);
    GroupMetadata getGroupMetadata(qint64 gid);

    QSqlQuery executeDatabaseQuery(const QString &query);
    void executeDatabaseQuery(QSqlQuery &query);
    const QList<Module *> &installedModules() {return mBot->mModules;}
    void sendMessage(qint64 id, bool chat, const QString &message, qint64 replyTo);
    void sendBroadcast(const QList<qint64> &users, const QString &message);
    void forwardMessage(qint64 id, bool chat, qint64 msgId);

    Model *newModel(const QString &section, const QString &name, qint64 version,
                    const QDate &versionDate);
    void registerModel(Model *model);
    Model *model(const QString &section, const QString &name);
};

#endif // BOTINTERFACE_H
