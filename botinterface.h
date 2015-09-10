#ifndef BOTINTERFACE_H
#define BOTINTERFACE_H

#include <QObject>
#include <QSqlQuery>

#include "bot.h"

class BotInterface : public QObject
{
    Q_OBJECT
private:
    Bot *mBot;

public:
    explicit BotInterface(Bot *bot, QObject *parent = 0);

    QSqlQuery executeDatabaseQuery(const QString &query);
    void executeDatabaseQuery(QSqlQuery &query);
    const QList<Module *> &installedModules() {return mBot->mModules;}
    void sendMessage(qint64 id, bool chat, const QString &message, qint64 replyTo);
};

#endif // BOTINTERFACE_H
