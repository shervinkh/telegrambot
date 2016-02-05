#include "binputmessage.h"
#include <QDateTime>
#include <QRegularExpression>

BInputMessage::BInputMessage()
{
    mId = 0;
    mChatId = 0;
    mUserId = 0;
    mForwardedFrom = 0;
    mReplyFromId = 0;
    mReplyFromUser = 0;
}

BInputMessage::BInputMessage(qint64 id, qint64 userId, qint64 chatId, qint64 date,
                             const QString &message, qint64 forwardedFrom, qint64 forwardedDate,
                             qint64 replyFromId, int messageMediaType, AccessLevel accessLevel)
{
    mId = id;
    mUserId = userId;
    mChatId = chatId;
    mDate = QDateTime::fromMSecsSinceEpoch(date * 1000).toUTC();
    mMessage = message;
    mForwardedFrom = forwardedFrom;
    mForwardedDate = QDateTime::fromMSecsSinceEpoch(forwardedDate * 1000).toUTC();
    mReplyFromId = replyFromId;
    mReplyFromUser = 0;
    mMessageMediaType = messageMediaType;
    mAccessLevel = accessLevel;
    mPM = (mChatId == 0);
}

void BInputMessage::setGroupChat(qint64 gid)
{
    mChatId = gid;
}

void BInputMessage::setReplyData(qint64 replyUser, const QString &replyMessage)
{
    mReplyFromUser = replyUser;
    mReplyFromMessage = replyMessage;
}

QString BInputMessage::command() const
{
    auto args = getArgumentsArray();

    if (args.size() > 0 && (args[0].toString().startsWith("!") || args[0].toString().startsWith("/")))
        return args[0].toString().mid(1);

    return QString();
}

QVariantList BInputMessage::getArgumentsArray() const
{
    auto args = mMessage.toLower().split(QRegularExpression("\\s+"), QString::SkipEmptyParts);

    QVariantList result;
    foreach (auto arg, args)
        result.append(arg);

    return result;
}

QString BInputMessage::getStringFromArgument(int whichArgument) const
{
    auto startPos = 0;
    for (int i = 0; i < whichArgument; i++)
    {
        auto nextWhiteSpace = mMessage.indexOf(QRegularExpression("\\s"), startPos);

        if (nextWhiteSpace == -1)
            return QString();

        startPos = mMessage.indexOf(QRegularExpression("\\S"), nextWhiteSpace);

        if (startPos == -1)
            return QString();
    }

    return mMessage.mid(startPos);
}
