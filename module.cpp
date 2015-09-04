#include "module.h"
#include "bot.h"

Module::Module(const QString name, const qint64 version, QObject *parent)
    : QObject(parent), mName(name.toLower()), mVersion(version), mBot(Q_NULLPTR),
      mLoggingCategoryName(QByteArray("bot.module.").append(mName)),
      mLoggingCategory(mLoggingCategoryName.data())
{

}

Module::~Module()
{

}

void Module::internalInit()
{

}

void Module::sendMessage(qint64 id, bool chat, const QString &message, qint64 replyTo)
{
    if (mBot)
        mBot->sendMessage(id, chat, message, replyTo);
}
