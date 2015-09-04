#include "module.h"
#include "bot.h"

Module::Module(const QString name, const qint64 version, QObject *parent)
    : QObject(parent), mName(name.toLower()), mVersion(version), mBot(Q_NULLPTR),
      mLoggingCategory(QString("bot.module.%1").arg(mName).toStdString().c_str())
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
