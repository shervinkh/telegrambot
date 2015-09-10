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
                             qint64 replyFromId, int messageMediaType)
{
    mId = id;
    mUserId = userId;
    mChatId = chatId;
    mDate = QDateTime::fromMSecsSinceEpoch(date * 1000);
    mMessage = message;
    mForwardedFrom = forwardedFrom;
    mForwardedDate = QDateTime::fromMSecsSinceEpoch(forwardedDate * 1000);
    mReplyFromId = replyFromId;
    mReplyFromUser = 0;
    mMessageMediaType = messageMediaType;
}

void BInputMessage::setReplyData(qint64 replyUser, const QString &replyMessage)
{
    mReplyFromUser = replyUser;
    mReplyFromMessage = replyMessage;
}

QStringList BInputMessage::getArgumentsArray()
{
    return mMessage.toLower().split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
}

QString BInputMessage::getStringFromArgument(int whichArgument)
{
    int startPos = 0;
    for (int i = 0; i < whichArgument; i++)
    {
        int nextWhiteSpace = mMessage.indexOf(QRegularExpression("\\s"), startPos);

        if (nextWhiteSpace == -1)
            return QString();

        startPos = mMessage.indexOf(QRegularExpression("\\S"), nextWhiteSpace);

        if (startPos == -1)
            return QString();
    }

    return mMessage.mid(startPos);
}
