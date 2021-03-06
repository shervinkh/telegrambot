#include "botinterface.h"
#include "botutils.h"
#include "module.h"
#include "core/core_model/coremodel.h"
#include "core/core_config/coreconfig.h"
#include "database.h"
#include "redis.h"

BotInterface::BotInterface(Bot *bot, QObject *parent)
                           : QObject(parent), mBot(bot)
{
    mMetadata = new Metadata(mBot->mMetaRedis, this);
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
        peer.setClassType(InputPeer::typeInputPeerUser);
        peer.setUserId(id);
        peer.setAccessHash(mBot->mMetaRedis->hget(QString("user#%1").arg(id), "access_hash").toLongLong());
    }

    return peer;
}

void BotInterface::sendMessage(qint64 id, bool chat, const QString &message, qint64 replyTo)
{
    if (message.isEmpty())
        return;

    mBot->mTelegram->messagesSendMessage(true, false, getPeer(id, chat), replyTo, message,
                                         BotUtils::secureRandomLong(), ReplyMarkup(),
                                         QList<MessageEntity>());
}

void BotInterface::sendBroadcast(const QList<qint64> &users, const QString &message)
{
    mBot->sendBroadcast(users, message);
}

void BotInterface::forwardBroadcast(const QList<qint64> &users, qint64 msgId)
{
    mBot->continueBroadcast(msgId, users);
}

void BotInterface::forwardMessage(qint64 id, bool chat, qint64 msgId)
{
    mBot->mTelegram->messagesForwardMessage(getPeer(id, chat), msgId, BotUtils::secureRandomLong());
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
