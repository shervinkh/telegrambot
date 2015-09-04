#include "bot.h"
#include "database.h"
#include "redis.h"
#include "module.h"
#include "botutils.h"
#include <QtCore>

const int Bot::METADATA_UPDATE_SLICE = 100;

Q_LOGGING_CATEGORY(BOT_CORE, "bot.core")

Bot::Bot(Database *database, Redis *redis, QObject *parent)
    : QObject(parent), input(stdin), output(stdout), mDatabase(database), mRedis(redis)
{
    mCurrentOperation = None;
    mLoggedIn = false;

    //FIXME: Change 40->50 in production
    mTelegram = new Telegram("149.154.167.50", 443, 2, 39006, "034ac9bc16b9b1dbae4e3f846e9f5dd9",
                            "+989212823848", "~/.telegrambot", "tg.pub");

    //Auth
    connect(mTelegram, &Telegram::authNeeded, this, &Bot::onAuthNeeded);
    connect(mTelegram, &Telegram::authCheckPhoneAnswer, this, &Bot::onAuthCheckPhoneAnswer);
    connect(mTelegram, &Telegram::authSendCodeAnswer, this, &Bot::onAuthSendCodeAnswer);
    connect(mTelegram, &Telegram::authLoggedIn, this, &Bot::onAuthLoggedIn);

    //Messages
    connect(mTelegram, &Telegram::messagesGetMessagesAnswer, this, &Bot::onMessagesGetMessagesAnswer);
    connect(mTelegram, &Telegram::messagesGetDialogsAnswer, this, &Bot::onMessagesGetDialogsAnswer);
    connect(mTelegram, &Telegram::messagesGetFullChatAnswer, this, &Bot::onMessagesGetFullChatAnswer);

    //Error
    connect(mTelegram, &Telegram::error, this, &Bot::onError);

    //Updates
    connect(mTelegram, &Telegram::updates, this, &Bot::onUpdates);
    connect(mTelegram, &Telegram::updatesCombined, this, &Bot::onUpdatesCombined);
    connect(mTelegram, &Telegram::updateShort, this, &Bot::onUpdateShort);
    connect(mTelegram, &Telegram::updateShortMessage, this, &Bot::onUpdateShortMessage);
    connect(mTelegram, &Telegram::updateShortChatMessage, this, &Bot::onUpdateShortChatMessage);
    connect(mTelegram, &Telegram::updatesTooLong, this, &Bot::onUpdatesTooLong);
}

//Start Metadata
void Bot::updateMetadata()
{
    if (mMetadataStart != -1)
        return;

    qCInfo(BOT_CORE) << "Getting basic groups and users metadata...";

    mRedis->del("groups");
    mRedis->del("users");
    mMetadataStart = 0;
    mTelegram->messagesGetDialogs(mMetadataStart, 0, METADATA_UPDATE_SLICE);
}

void Bot::updateUserGroupLinks()
{
    if (mLinkdataCount != -1)
        return;

    qCInfo(BOT_CORE) << "Getting group-user links...";

    QList<QVariant> users = mRedis->smembers("users").toList();
    foreach (QVariant uid, users)
        mRedis->del(QString("user#%1:groups").arg(uid.toLongLong()));

    QList<QVariant> groups = mRedis->smembers("groups").toList();
    mLinkdataCount = groups.size();

    foreach (QVariant gid, groups)
    {
        mRedis->del(QString("chat#%1:users").arg(gid.toLongLong()));
        mTelegram->messagesGetFullChat(gid.toLongLong());
    }
}
//End Metadata

//Start DataGetter
void Bot::getUserData(const User &user)
{
    if (user.classType() != User::typeUserDeleted && user.classType() != User::typeUserEmpty)
    {
        mRedis->sadd("users", user.id());
        QString dataKey = QString("user#%1").arg(user.id());
        mRedis->hset(dataKey, "first_name", user.firstName());
        mRedis->hset(dataKey, "last_name", user.lastName());
        mRedis->hset(dataKey, "username", user.username());
        mRedis->hset(dataKey, "access_hash", user.accessHash());
    }
}

void Bot::getChatData(const Chat &chat)
{
    if (chat.classType() != Chat::typeChatForbidden && chat.classType() != Chat::typeChatEmpty && !chat.left())
    {
        mRedis->sadd("groups", chat.id());
        QString dataKey = QString("chat#%1").arg(chat.id());
        mRedis->hset(dataKey, "title", chat.title());
    }
}

void Bot::getChatParticipantsData(const ChatParticipants &chatParticipants)
{
    if (chatParticipants.classType() != ChatParticipants::typeChatParticipantsForbidden)
    {
        qint64 chatId = chatParticipants.chatId();

        QString chatDataKey = QString("chat#%1").arg(chatId);
        mRedis->hset(chatDataKey, "admin", chatParticipants.adminId());

        QString chatMembersDataKey = QString("chat#%1:users").arg(chatId);
        foreach (ChatParticipant chatParticipant, chatParticipants.participants())
        {
            mRedis->hset(chatMembersDataKey, QString::number(chatParticipant.userId()), chatParticipant.inviterId());
            mRedis->sadd(QString("user#%1:groups").arg(chatParticipant.userId()), chatId);
        }
    }
}
//End DataGetter

//Start Auth
void Bot::onAuthNeeded()
{
    output << "Authentication Needed!" << endl << flush;
    mCurrentOperation = AuthCheckPhone;
    mTelegram->authCheckPhone();
}

void Bot::onAuthCheckPhoneAnswer(qint64 id, bool phoneRegistered)
{
    Q_UNUSED(id);

    mCurrentOperation = None;
    qCInfo(BOT_CORE) << "Phone Checked: " << phoneRegistered;

    if (!phoneRegistered)
        qFatal("Phone number's not registered on telegram");

    mTelegram->authSendCode();
}

void Bot::onAuthSendCodeAnswer(qint64 id, bool phoneRegistered, qint32 sendCallTimeout)
{
    Q_UNUSED(id);
    Q_UNUSED(phoneRegistered);
    Q_UNUSED(sendCallTimeout);

    output << "Please enter the code you got from telegram: " << endl << flush;

    QString code;
    input >> code;

    mTelegram->authSignIn(code);
}

void Bot::onAuthLoggedIn()
{
    mLoggedIn = true;
    qCInfo(BOT_CORE) << "Logged In.";
    mTelegram->accountRegisterDevice(QCoreApplication::applicationName(), QCoreApplication::applicationVersion());
    updateMetadata();
}
//End Auth

//Start Messages
void Bot::onMessagesGetMessagesAnswer(qint64 id, qint32 sliceCount, const QList<Message> &messages,
                                      const QList<Chat> &chats, const QList<User> &users)
{
    qCDebug(BOT_CORE) << "Messages Get Answer: id=" << id << ", sliceCount=" << sliceCount;
    foreach (Message message, messages)
        qCDebug(BOT_CORE) << "Message: id=" << message.id() << ", message=" << message.message();

    foreach (Chat chat, chats)
        qCDebug(BOT_CORE) << "Chat id=" << chat.id() << ", name=" << chat.title();

    foreach (User user, users)
        qCDebug(BOT_CORE) << "User id=" << user.id() << ", name=" << user.firstName() << " " << user.lastName();

    if (!messages.isEmpty())
    {
        Message message = messages.first();
        qint64 messageId = message.id();
        if (pendingForReplyFromQueue.contains(messageId))
        {
            MessageList pendings = pendingForReplyFromQueue[messageId];
            pendingForReplyFromQueue.remove(messageId);

            foreach (BInputMessage curMessage, pendings)
            {
                curMessage.setReplyData(message.fromId(), message.message());
                eventNewMessage(curMessage);
            }
        }
    }
}

void Bot::onMessagesGetDialogsAnswer(qint64 id, qint32 sliceCount, const QList<Dialog> &dialogs, const QList<Message> &messages,
                                     const QList<Chat> &chats, const QList<User> &users)
{
    Q_UNUSED(id);
    Q_UNUSED(dialogs);
    Q_UNUSED(messages);

    int count = dialogs.size();

    foreach (Chat c, chats)
        getChatData(c);

    foreach (User u, users)
        getUserData(u);

    if (mMetadataStart + count < sliceCount)
    {
        mMetadataStart += count;
        mTelegram->messagesGetDialogs(mMetadataStart, 0, METADATA_UPDATE_SLICE);
    }
    else
    {
        mMetadataStart = -1;
        qCInfo(BOT_CORE) << "Fininshed getting basic groups and users metadata!";
        updateUserGroupLinks();
    }
}

void Bot::onMessagesGetFullChatAnswer(qint64 id, const ChatFull &chatFull, const QList<Chat> &chats, const QList<User> &users)
{
    Q_UNUSED(id);
    Q_UNUSED(chats);
    Q_UNUSED(users);

    getChatParticipantsData(chatFull.participants());

    if (--mLinkdataCount == 0) {
        qCInfo(BOT_CORE) << "Fininshed getting group-user links!";
        mLinkdataCount = -1;
    }
}
//End Messages

//Start Error
void Bot::onError(qint64 id, qint32 errorCode, QString errorText)
{
    Q_UNUSED(id);
    Q_UNUSED(errorText);

    if (mCurrentOperation == AuthCheckPhone && errorCode == 400)
        qFatal("Invalid phone number.");
}
//End Error

//Start Updates
void Bot::onUpdates(QList<Update> updates, QList<User> users, QList<Chat> chats, qint32 date, qint32 seq)
{
    qCDebug(BOT_CORE) << "Updates: " << endl << flush;
    foreach (Update update, updates)
    {
        QString type;
        switch (update.message().media().classType())
        {
            case MessageMedia::typeMessageMediaAudio:
                type = "audio";
                break;
            case MessageMedia::typeMessageMediaContact:
                type = "contact";
                break;
            case MessageMedia::typeMessageMediaDocument:
                type = "document";
                break;
            case MessageMedia::typeMessageMediaEmpty:
                type = "empty";
                break;
            case MessageMedia::typeMessageMediaGeo:
                type = "geo";
                break;
            case MessageMedia::typeMessageMediaPhoto:
                type = "photo";
                break;
            case MessageMedia::typeMessageMediaVideo:
                type = "video";
                break;
            default:
                type = "N/S";
                break;
        }

        qCDebug(BOT_CORE) << "--Update: class=" << updateCode(update.classType()) << ", chatId=" << update.chatId() <<
                  ", userId=" << update.userId() << ", message=" << update.message().message() << ' '<<
                  ", type=" << type << ", len=" << update.message().media().bytes().length();
    }

    foreach (Chat chat, chats)
        qCDebug(BOT_CORE) << "--Chat: id=" << chat.id() << ", title=" << chat.title();

    foreach (User user, users)
        qCDebug(BOT_CORE) << "--User: id=" << user.id() << ", name=" << user.firstName() << ' ' << user.lastName();

    qCDebug(BOT_CORE) << "Date=" << date << ", seq=" << seq;

}

void Bot::onUpdatesCombined(QList<Update> updates, QList<User> users, QList<Chat> chats, qint32 date,
                            qint32 seqStart, qint32 seq)
{
    qCDebug(BOT_CORE) << "Updates Combined: ";
    foreach (Update update, updates)
        qCDebug(BOT_CORE) << "--Update: class=" << updateCode(update.classType()) << ", chatId=" << update.chatId() <<
                  ", userId=" << update.userId() << ", message=" << update.message().message();

    foreach (Chat chat, chats)
        qCDebug(BOT_CORE) << "--Chat: id=" << chat.id() << ", title=" << chat.title();

    foreach (User user, users)
        qCDebug(BOT_CORE) << "--User: id=" << user.id() << ", name=" << user.firstName() << ' ' << user.lastName();

    qCDebug(BOT_CORE) << "Date=" << date << ", seqStart=" << seqStart << ", seq=" << seq;
}

void Bot::onUpdateShort(Update update, qint32 date)
{
    qCDebug(BOT_CORE) << "Update Short: class=" << updateCode(update.classType()) << ", date=" << date;
    if (update.classType() == Update::typeUpdateUserStatus)
        qCDebug(BOT_CORE) << "Details: user=" << update.userId() << ", username=" << update.username() << ", status=" <<
                  ((update.status().classType() == UserStatus::typeUserStatusOnline) ? "Online" :
                  ((update.status().classType() == UserStatus::typeUserStatusOffline) ? "Offline" : "Unknown"))
               << ", wasOnline=" << update.status().wasOnline();
}

void Bot::onUpdateShortMessage(qint32 id, qint32 userid, const QString &message, qint32 pts,
                               qint32 pts_count, qint32 date, qint32 fwd_from_id, qint32 fwd_date,
                               qint32 reply_to_msg_id, bool unread, bool out)
{
    qCDebug(BOT_CORE) << "Update Short Message: id=" << id << ", userId=" << userid << ", message=" << message <<
              ", pts=" << pts << ", pts_count=" << pts_count << ", date=" << date << ", fwd_from_id=" <<
              fwd_from_id << ", fwd_date=" << fwd_date << ", reply_to_msg_id=" << reply_to_msg_id <<
              ", unread=" << unread << ", out=" << out;

    BInputMessage newMessage(id, userid, 0, date, message, fwd_from_id, fwd_date, reply_to_msg_id);
    if (newMessage.replyFromId() != 0)
        resolveReplyFrom(newMessage);
    else
        eventNewMessage(newMessage);
}

void Bot::onUpdateShortChatMessage(qint32 id, qint32 fromId, qint32 chatId, const QString &message,
                                   qint32 pts, qint32 pts_count, qint32 date, qint32 fwd_from_id,
                                   qint32 fwd_date, qint32 reply_to_msg_id, bool unread, bool out)
{
    qCDebug(BOT_CORE) << "Update Short Chat Message: id=" << id << ", fromId=" << fromId << ", chatId=" << chatId <<
              ", message=" << message << ", pts=" << pts << ", pts_count=" << pts_count << ", date=" <<
              date << ", fwd_from_id=" << fwd_from_id << ", fwd_date=" << fwd_date << ", reply_to_msg_id=" <<
              reply_to_msg_id << ", unread=" << unread << ", out=" << out;

    BInputMessage newMessage(id, fromId, chatId, date, message, fwd_from_id, fwd_date, reply_to_msg_id);
    if (newMessage.replyFromId() != 0)
        resolveReplyFrom(newMessage);
    else
        eventNewMessage(newMessage);
}

void Bot::onUpdatesTooLong()
{
    qCWarning(BOT_CORE) << "Updates too long :|" << endl << flush;
}

QString Bot::updateCode(int code)
{
    switch (code){
    case Update::typeUpdateActivation:
        return "Activation";
    case Update::typeUpdateChatParticipantAdd:
        return "Chat Participant Add";
    case Update::typeUpdateChatParticipantDelete:
        return "Chat Participant Delete";
    case Update::typeUpdateChatParticipants:
        return "Chat Participants";
    case Update::typeUpdateChatUserTyping:
        return "Chat User Typing";
    case Update::typeUpdateContactLink:
        return "Contacts Link";
    case Update::typeUpdateContactRegistered:
        return "Contacts Registered";
    case Update::typeUpdateDcOptions:
        return "Dc Options";
    case Update::typeUpdateDeleteMessages:
        return "Delete Messages";
    case Update::typeUpdateEncryptedChatTyping:
        return "Encrypted Chat Typing";
    case Update::typeUpdateEncryptedMessagesRead:
        return "Encrypted Message Read";
    case Update::typeUpdateEncryption:
        return "Encryption";
    case Update::typeUpdateInvalid:
        return "Invalid";
    case Update::typeUpdateMessageID:
        return "Message ID";
    case Update::typeUpdateNewAuthorization:
        return "New Authorization";
    case Update::typeUpdateNewEncryptedMessage:
        return "New Encrypted Message";
    case Update::typeUpdateNewGeoChatMessage:
        return "New Geo Chat Message";
    case Update::typeUpdateNewMessage:
        return "New Message";
    case Update::typeUpdateNotifySettings:
        return "Notify Settings";
    case Update::typeUpdatePrivacy:
        return "Privacy";
    case Update::typeUpdateReadHistoryInbox:
        return "Read History Inbox";
    case Update::typeUpdateReadHistoryOutbox:
        return "Read History Outbox";
    case Update::typeUpdateReadMessages:
        return "Read Messages";
    case Update::typeUpdateRestoreMessages:
        return "Restore Messages";
    case Update::typeUpdateServiceNotification:
        return "Service Notification";
    case Update::typeUpdateUserBlocked:
        return "User Blocked";
    case Update::typeUpdateUserName:
        return "User Name";
    case Update::typeUpdateUserPhone:
        return "User Phone";
    case Update::typeUpdateUserPhoto:
        return "User Photo";
    case Update::typeUpdateUserStatus:
        return "User Status";
    case Update::typeUpdateUserTyping:
        return "User Typing";
    default:
        return "Unknown";
    }
}

//End Updates

//Start Utilities

//Resolve Reply From
void Bot::resolveReplyFrom(BInputMessage message)
{
    qint64 replyFrom = message.replyFromId();

    if (pendingForReplyFromQueue.contains(replyFrom))
        pendingForReplyFromQueue[replyFrom].append(message);
    else {
        pendingForReplyFromQueue[replyFrom].append(message);

        QList<qint32> ids;
        ids.append(replyFrom);
        mTelegram->messagesGetMessages(ids);
    }
}

//End Utilities

//Start Events

void Bot::eventNewMessage(BInputMessage message)
{
    qCDebug(BOT_CORE) << "Event Here!!!!!!";
    qCDebug(BOT_CORE) << "id= " << message.Id() << ", user=" << message.userId() << ", chat=" << message.chatId() <<
              ", message=" << message.message() << ", date=" << message.date().toString() << ", fwdFrom=" << message.forwardedFrom() << ", fwdDate=" <<
              message.forwardedDate().toString() << "Reply From: " << message.replyFromId() << ", " << message.replyFromUser() <<
              ", " << message.replyFromMessage();

    foreach (Module *module, mModules)
        module->onNewMessage(message);
}

//End Events

//Start Module System

void Bot::addModule(Module *module)
{
    module->setBot(this);
    mModules.append(module);
}

void Bot::init()
{
    foreach (Module *module, mModules)
    {
        module->internalInit();
        module->init();
    }

    mTelegram->init();
}

//End Module System

//Start Module Interface
void Bot::sendMessage(qint64 id, bool chat, const QString &message, qint64 replyTo)
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
        peer.setAccessHash(mRedis->hget(QString("user#%1").arg(id), "access_hash").toLongLong());
    }

    mTelegram->messagesSendMessage(peer, BotUtils::secureRandomLong(), message, replyTo);
}
//End Module Interface
