#ifndef BMESSAGE_H
#define BMESSAGE_H

#include <QObject>
#include <QDateTime>

class BInputMessage
{
public:
    enum AccessLevel { User, Admin, Superuser };

private:
    qint64 mId;
    qint64 mUserId;
    qint64 mChatId;
    QDateTime mDate;
    QString mMessage;
    qint64 mForwardedFrom;
    QDateTime mForwardedDate;
    qint64 mReplyFromId;
    qint64 mReplyFromUser;
    QString mReplyFromMessage;
    int mMessageMediaType;
    AccessLevel mAccessLevel;

public:

    BInputMessage();
    BInputMessage(qint64 id, qint64 userId, qint64 chatId, qint64 date, const QString &message,
                  qint64 forwardedFrom, qint64 forwardedDate, qint64 replyFromId, int messageMediaType,
                  AccessLevel accessLevel);

    QVariantList getArgumentsArray() const;
    QString command() const;
    QString getStringFromArgument(int whichArgument) const;

    void setReplyData(qint64 replyUser, const QString &replyMessage);

    qint64 id() const { return mId; }
    qint64 userId() const { return mUserId; }
    qint64 chatId() const { return mChatId; }
    QDateTime date() const { return mDate; }
    QString message() const { return mMessage; }
    qint64 forwardedFrom() const { return mForwardedFrom; }
    QDateTime forwardedDate() const { return mForwardedDate; }
    qint64 replyFromId() const { return mReplyFromId; }
    qint64 replyFromUser() const { return mReplyFromUser; }
    QString replyFromMessage() const { return mReplyFromMessage; }
    int messageMediaType() const { return mMessageMediaType; }
    bool isAdmin() const { return mAccessLevel == Admin || mAccessLevel == Superuser; }
    bool isSuperuser() const { return mAccessLevel == Superuser; }
};

#endif // BMESSAGE_H
