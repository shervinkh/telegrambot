#include "botinterface.h"
#include "botutils.h"
#include "module.h"
#include "core/core_model/coremodel.h"
#include "core/core_config/coreconfig.h"
#include "database.h"
#include "redis.h"

#include <QMetaProperty>

BotInterface::BotInterface(Bot *bot, QObject *parent)
                           : QObject(parent), mBot(bot)
{

}

QString BotInterface::aboutText()
{
    return mBot->aboutText();
}

bool BotInterface::debug()
{
    return qgetenv("DEBUG") == "true";
}

Module *BotInterface::getModule(const QString &name)
{
    foreach (auto module, installedModules())
        if (module->name() == name)
            return module;

    return Q_NULLPTR;
}

GroupMetadata BotInterface::getGroupMetadata(qint64 gid)
{
    if (!mBot->mMetaRedis->exists(QString("chat#%1").arg(gid)).toBool())
        return GroupMetadata();

    auto title = mBot->mMetaRedis->hget(QString("chat#%1").arg(gid), "title").toString();
    auto adminId = mBot->mMetaRedis->hget(QString("chat#%1").arg(gid), "admin").toLongLong();

    return GroupMetadata(gid, title, adminId);
}

void BotInterface::executeDatabaseQuery(QSqlQuery &query)
{
    mBot->mDatabase->execute(query);
}

QSqlQuery BotInterface::executeDatabaseQuery(const QString &query)
{
    return mBot->mDatabase->execute(query);
}

InputPeer BotInterface::getPeer(qint64 id, bool chat)
{
    InputPeer peer;

    if (chat)
    {
        peer.setClassType(InputPeer::typeInputPeerChat);
        peer.setChatId(id);
    }
    else
    {
        peer.setClassType(InputPeer::typeInputPeerForeign);
        peer.setUserId(id);
        peer.setAccessHash(mBot->mMetaRedis->hget(QString("user#%1").arg(id), "access_hash").toLongLong());
    }

    return peer;
}

void BotInterface::sendMessage(qint64 id, bool chat, const QString &message, qint64 replyTo)
{
    if (message.isEmpty())
        return;

    mBot->mTelegram->messagesSendMessage(getPeer(id, chat), BotUtils::secureRandomLong(), message, replyTo);
}

void BotInterface::sendBroadcast(const QList<qint64> &users, const QString &message)
{
    mBot->sendBroadcast(users, message);
}

void BotInterface::forwardMessage(qint64 id, bool chat, qint64 msgId)
{
    mBot->mTelegram->messagesForwardMessage(getPeer(id, chat), msgId);
}

Model *BotInterface::newModel(const QString &section, const QString &name, qint64 version, const QDate &versionDate)
{
    return new Model(this, section, name, version, versionDate);
}

void BotInterface::registerModel(Model *model)
{
    mBot->mCoreModel->registerModel(model);
}

Model *BotInterface::model(const QString &section, const QString &name)
{
    return mBot->mCoreModel->model(section, name);
}

BotConfig *BotInterface::config(const QString &name)
{
    return mBot->mCoreConfig->config(name);
}

void BotInterface::registerConfig(BotConfig *botConfig)
{
    mBot->mCoreConfig->registerConfig(botConfig);
}
