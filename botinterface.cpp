#include "botinterface.h"
#include "botutils.h"
#include "database.h"
#include "redis.h"

BotInterface::BotInterface(Bot *bot, QObject *parent)
                           : QObject(parent), mBot(bot)
{

}

void BotInterface::executeDatabaseQuery(QSqlQuery &query)
{
    mBot->mDatabase->execute(query);
}

QSqlQuery BotInterface::executeDatabaseQuery(const QString &query)
{
    return mBot->mDatabase->execute(query);
}

void BotInterface::sendMessage(qint64 id, bool chat, const QString &message, qint64 replyTo)
{
    if (message.isEmpty())
        return;

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
        peer.setAccessHash(mBot->mRedis->hget(QString("user#%1").arg(id), "access_hash").toLongLong());
    }

    mBot->mTelegram->messagesSendMessage(peer, BotUtils::secureRandomLong(), message, replyTo);
}
