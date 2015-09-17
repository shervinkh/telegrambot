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

    QString aboutText();
    bool debug();

    Module *getModule(const QString &name);

    QSqlQuery executeDatabaseQuery(const QString &query);
    void executeDatabaseQuery(QSqlQuery &query);
    const QList<Module *> &installedModules() {return mBot->mModules;}
    void sendMessage(qint64 id, bool chat, const QString &message, qint64 replyTo);

    void registerModel(const QString &section, QObject *model);
    QString getModelDatabaseTable(QObject *object);
    int saveModelObject(QObject *object);
    int deleteModelObject(QObject *object);
};

#endif // BOTINTERFACE_H
