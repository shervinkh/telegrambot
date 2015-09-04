#include "binputmessage.h"
#include <QDateTime>

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
                             qint64 replyFromId)
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
}
