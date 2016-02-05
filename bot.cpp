#include "bot.h"
#include "database.h"
#include "core/core_data_store/coredatastore.h"
#include "core/core_model/coremodel.h"
#include "core/core_config/coreconfig.h"
#include "redis.h"
#include "module.h"
#include "botinterface.h"
#include <QtCore>
#include <QStandardPaths>

const QString Bot::sVersion = "0.0.1";
const QDate Bot::sVersionDate = QDate(2015, 9, 11);

const int Bot::METADATA_UPDATE_SLICE = 100;

Q_LOGGING_CATEGORY(BOT_CORE, "bot.core")

Bot::Bot(Database *database, qint64 superuserId, QObject *parent)
    : QObject(parent), input(stdin), output(stdout), mSuperuserId(superuserId), mDatabase(database)
{
    mCurrentOperation = None;
    mLoggedIn = false;
    mBotInterface = new BotInterface(this, this);
    mCoreModel = new CoreModel(mBotInterface, this);
    mCoreModel->init();
    mCoreConfig = new CoreConfig(mBotInterface, this);
    mCoreDataStore = new CoreDataStore(mBotInterface, this);
    mCoreRedis = mCoreDataStore->redis(CoreDataStore::CoreRedis);
    mMetaRedis = mCoreDataStore->redis(CoreDataStore::MetaRedis);

    QString homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).last();

    //FIXME: Change 40->50 in production
    mTelegram = new Telegram("149.154.167.50", 443, 2, 39006, "034ac9bc16b9b1dbae4e3f846e9f5dd9",
                            "+989212823848", homeDir + "/.telegrambot", homeDir + "/tg.pub");

    //Auth
    connect(mTelegram, &Telegram::authNeeded, this, &Bot::onAuthNeeded);
    connect(mTelegram, &Telegram::authCheckPhoneAnswer, this, &Bot::onAuthCheckPhoneAnswer);
    connect(mTelegram, &Telegram::authSendCodeAnswer, this, &Bot::onAuthSendCodeAnswer);
    connect(mTelegram, &Telegram::authLoggedIn, this, &Bot::onAuthLoggedIn);
    connect(mTelegram, &Telegram::authSignInError, this, &Bot::onAuthSignInError);

    //Messages
    connect(mTelegram, &Telegram::messagesGetMessagesAnswer, this, &Bot::onMessagesGetMessagesAnswer);
    connect(mTelegram, &Telegram::messagesGetDialogsAnswer, this, &Bot::onMessagesGetDialogsAnswer);
    connect(mTelegram, &Telegram::messagesGetFullChatAnswer, this, &Bot::onMessagesGetFullChatAnswer);
    connect(mTelegram, &Telegram::messagesSendMessageAnswer, this, &Bot::onMessagesSendMessageAnswer);

    //Error
    connect(mTelegram, &Telegram::error, this, &Bot::onError);

    //Updates
    connect(mTelegram, &Telegram::updates, this, &Bot::onUpdates);
    connect(mTelegram, &Telegram::updatesCombined, this, &Bot::onUpdatesCombined);
    connect(mTelegram, &Telegram::updateShort, this, &Bot::onUpdateShort);
    connect(mTelegram, &Telegram::updateShortMessage, this, &Bot::onUpdateShortMessage);
    connect(mTelegram, &Telegram::updateShortChatMessage, this, &Bot::onUpdateShortChatMessage);
    connect(mTelegram, &Telegram::updatesTooLong, this, &Bot::onUpdatesTooLong);

    mTimer = new QTimer(this);
    connect(mTimer, &QTimer::timeout, this, &Bot::cronTask);
}

//Start Metadata
void Bot::updateMetadata()
{
    if (mMetadataStart != -1)
        return;

    qCInfo(BOT_CORE) << "Getting basic groups and users metadata...";

    mMetaRedis->del("groups");
    mMetaRedis->del("users");
    mMetadataStart = 0;
    updateNextMetadata();
}

void Bot::updateNextMetadata()
{
    QTimer::singleShot(1500, this, &Bot::updateNextMetadataImp);
}

void Bot::updateNextMetadataImp()
{
    mTelegram->messagesGetDialogs(mMetadataStart, 0, METADATA_UPDATE_SLICE);
}

void Bot::updateUserGroupLinks()
{
    if (!mLinkdataList.isEmpty())
        return;

    qCInfo(BOT_CORE) << "Getting group-user links...";

    auto users = mMetaRedis->smembers("users").toList();
    foreach (auto uid, users)
        mMetaRedis->del(QString("user#%1.groups").arg(uid.toLongLong()));

    auto groups = mMetaRedis->smembers("groups").toList();
    foreach (auto gid, groups)
        mLinkdataList.append(gid.toLongLong());

    updateNextGroupLinks();
}

void Bot::updateNextGroupLinks()
{
    QTimer::singleShot(1000, this, &Bot::updateNextGroupLinksImp);
}

void Bot::updateNextGroupLinksImp()
{
    qCInfo(BOT_CORE) << "Remaining groups: " << mLinkdataList.size();

    if (!mLinkdataList.isEmpty())
    {
        auto nextGid = mLinkdataList.takeFirst();
        mTelegram->messagesGetFullChat(nextGid);
    }
    else
        qCInfo(BOT_CORE) << "Fininshed getting group-user links!";
}

BInputMessage::AccessLevel Bot::userAccessLevel(qint64 gid, qint64 uid)
{
    if (uid == mSuperuserId)
        return BInputMessage::Superuser;
    else if (uid == mBotInterface->getGroupMetadata(gid).adminId())
        return BInputMessage::Admin;

    return BInputMessage::User;
}

//End Metadata

//Start DataGetter
void Bot::eatUserData(const User &user)
{
    if (user.classType() != User::typeUserDeleted && user.classType() != User::typeUserEmpty)
    {
        qCDebug(BOT_CORE) << "Eating user: " << user.id();
        mMetaRedis->sadd("users", user.id());
        auto dataKey = QString("user#%1").arg(user.id());
        mMetaRedis->hset(dataKey, "first_name", user.firstName());
        mMetaRedis->hset(dataKey, "last_name", user.lastName());
        mMetaRedis->hset(dataKey, "username", user.username());
        mMetaRedis->hset(dataKey, "access_hash", user.accessHash());
    }
}

void Bot::eatUserDataChange(qint64 id, const QString &firstName,
                            const QString &lastName, const QString &username)
{
    auto dataKey = QString("user#%1").arg(id);
    if (mMetaRedis->exists(dataKey).toBool())
    {
        qCDebug(BOT_CORE) << "Eating user change: " << id;
        mMetaRedis->hset(dataKey, "first_name", firstName);
        mMetaRedis->hset(dataKey, "last_name", lastName);
        mMetaRedis->hset(dataKey, "username", username);
    }
}

void Bot::eatChatData(const Chat &chat)
{
    if (chat.classType() != Chat::typeChatForbidden && chat.classType() != Chat::typeChatEmpty && !chat.left())
    {
        qCDebug(BOT_CORE) << "Eating chat meta: " << chat.id();
        mMetaRedis->sadd("groups", chat.id());
        auto dataKey = QString("chat#%1").arg(chat.id());
        mMetaRedis->hset(dataKey, "title", chat.title());
    }
}

void Bot::eatChatParticipantsData(const ChatParticipants &chatParticipants)
{
    if (chatParticipants.classType() != ChatParticipants::typeChatParticipantsForbidden)
    {
        auto chatId = chatParticipants.chatId();
        qCDebug(BOT_CORE) << "Eating chat full: " << chatId;

        auto chatDataKey = QString("chat#%1").arg(chatId);
        mMetaRedis->hset(chatDataKey, "admin", chatParticipants.adminId());

        auto chatMembersDataKey = QString("chat#%1.users").arg(chatId);
        mMetaRedis->del(chatMembersDataKey);
        foreach (ChatParticipant chatParticipant, chatParticipants.participants())
        {
            mMetaRedis->hset(chatMembersDataKey, QString::number(chatParticipant.userId()), chatParticipant.inviterId());
            mMetaRedis->sadd(QString("user#%1.groups").arg(chatParticipant.userId()), chatId);
        }
    }
}

void Bot::eatDelUser(qint64 gid, qint64 uid)
{
    qCDebug(BOT_CORE) << "Eating user del: " << uid << "from" << gid;
    mMetaRedis->srem(QString("user#%1.groups").arg(uid), gid);
    mMetaRedis->hdel(QString("chat#%1.users").arg(gid), uid);
}

void Bot::eatChangeTitle(qint64 gid, const QString &title)
{
    qCDebug(BOT_CORE) << "Eating change title: " << gid;
    auto chatDataKey = QString("chat#%1").arg(gid);
    if (mMetaRedis->exists(chatDataKey).toBool())
        mMetaRedis->hset(chatDataKey, "title", title);
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

    startCron();

    if (!mBotInterface->debug())
        updateMetadata();
    else
        qCInfo(BOT_CORE) << "Not updating metadata due to running in debug mode";
}

void Bot::onAuthSignInError(qint64 id, qint32 errorCode, const QString &errorText)
{
    Q_UNUSED(id);

    qCWarning(BOT_CORE) << "SignInError happened: " << errorCode << errorText;
}

//End Auth

//Start Messages
void Bot::onMessagesGetMessagesAnswer(qint64 id, qint32 sliceCount, const QList<Message> &messages,
                                      const QList<Chat> &chats, const QList<User> &users)
{
    Q_UNUSED(id)
    Q_UNUSED(sliceCount)
    Q_UNUSED(chats)
    Q_UNUSED(users)

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
    Q_UNUSED(messages);

    qCInfo(BOT_CORE) << QString("Got entries %1 - %2 of total %3")
                        .arg(mMetadataStart + 1).arg(mMetadataStart + dialogs.size()).arg(sliceCount);

    int count = dialogs.size();

    foreach (Chat c, chats)
        eatChatData(c);

    foreach (User u, users)
        eatUserData(u);

    if (mMetadataStart + count < sliceCount)
    {
        mMetadataStart += count;
        updateNextMetadata();
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
    foreach (auto user, users)
        eatUserData(user);

    eatChatParticipantsData(chatFull.participants());

    updateNextGroupLinks();
}

void Bot::onMessagesSendMessageAnswer(qint64 id, qint32 msgId, qint32 date, const MessageMedia &media, qint32 pts,
                                      qint32 pts_count, qint32 seq, const QList<ContactsLink> &links)
{
    Q_UNUSED(date)
    Q_UNUSED(pts)
    Q_UNUSED(media)
    Q_UNUSED(pts_count)
    Q_UNUSED(seq)
    Q_UNUSED(links)

    if (mBroadcastUsers.contains(id))
    {
        auto usersList = mBroadcastUsers[id];
        mBroadcastUsers.remove(id);
        continueBroadcast(msgId, usersList);
    }
}
//End Messages

//Start Broadcast
void Bot::sendBroadcast(const QList<qint64> &users, const QString &message)
{
    InputPeer me;
    me.setClassType(InputPeer::typeInputPeerSelf);

    auto reqId = mTelegram->messagesSendMessage(me, BotUtils::secureRandomLong(), message);
    mBroadcastUsers[reqId] = users;
}

void Bot::continueBroadcast(qint64 msgId, const QList<qint64> &users)
{
    foreach (auto user, users)
    {
        InputPeer peer;
        peer.setClassType(InputPeer::typeInputPeerForeign);
        peer.setUserId(user);
        peer.setAccessHash(mMetaRedis->hget(QString("user#%1").arg(user), "access_hash").toLongLong());

        mTelegram->messagesForwardMessage(peer, msgId);
    }
}
//End Broadcast

//Start Stated Messages
QString Bot::decodeMessageAction(MessageAction state)
{
    QString type;
    switch (state.classType())
    {
        case MessageAction::typeMessageActionChatAddUser:
            type = "Add User";
            break;
        case MessageAction::typeMessageActionChatCreate:
            type = "Create Chat";
            break;
        case MessageAction::typeMessageActionChatDeletePhoto:
            type = "Delete Photo";
            break;
        case MessageAction::typeMessageActionChatDeleteUser:
            type = "Delete User";
            break;
        case MessageAction::typeMessageActionChatEditPhoto:
            type = "Edit Photo";
            break;
        case MessageAction::typeMessageActionChatEditTitle:
            type = "Edit Title";
            break;
        case MessageAction::typeMessageActionEmpty:
            type = "Empty";
            break;
        case MessageAction::typeMessageActionGeoChatCheckin:
            type = "GeoChat Checkin";
            break;
        case MessageAction::typeMessageActionGeoChatCreate:
            type = "GeoChat Create";
            break;
        default:
            type = "N/S";
            break;
    }

    qCDebug(BOT_CORE) << ", title=" << state.title() << ", photo=" << state.photo().id() << ", userId=" << state.userId() <<
                         "Users: " << state.users();

    return type;
}
//End Stated Messages

//Start Error
void Bot::onError(qint64 id, qint32 errorCode, QString errorText)
{
    Q_UNUSED(id);

    qCWarning(BOT_CORE) << "Telegram Error happened: " << errorCode << errorText;

    if (mCurrentOperation == AuthCheckPhone && errorCode == 400)
        qFatal("Invalid phone number.");
}
//End Error

//Start Updates
void Bot::onUpdates(QList<Update> updates, QList<User> users, QList<Chat> chats, qint32 date, qint32 seq)
{
    qCDebug(BOT_CORE) << "Updates: " << endl << flush;

    foreach (auto user, users)
        eatUserData(user);

    foreach (auto chat, chats)
        eatChatData(chat);

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

        qCDebug(BOT_CORE) << "--Update: class=" << updateCode(update.classType()) << ", chatId=" << (chats.isEmpty() ? 0 : chats.first().id()) <<
                  ", userId=" << update.userId() << ", message=" << update.message().message() << ' '<<
                  ", type=" << type << ", id=" << update.message().id()
                          << ", Action= " << decodeMessageAction(update.message().action()) <<
                             ", participants: " << update.participants().participants().size();

        if (update.classType() == Update::typeUpdateNewMessage)
        {
            auto message = update.message();
            auto chatId = chats.isEmpty() ? 0 : chats.first().id();
            auto doer = message.fromId();
            auto doee = message.action().userId();

            if (message.action().classType() == MessageAction::typeMessageActionChatCreate)
                eventCreateChat(chatId, doer);
            else if (message.action().classType() == MessageAction::typeMessageActionChatAddUser)
                eventAddUser(chatId, doee, doer);
            else if (message.action().classType() == MessageAction::typeMessageActionChatJoinedByLink)
                eventAddUser(chatId, doer, doer);
            else if (message.action().classType() == MessageAction::typeMessageActionChatDeleteUser)
                eventDelUser(chatId, doee, doer);
            else if (message.action().classType() == MessageAction::typeMessageActionChatEditTitle)
                eventChangeTitle(chatId, message.action().title(), doer);
            else
            {
                auto gid = chats.isEmpty() ? 0 : chats.first().id();
                auto accessLevel = userAccessLevel(gid, message.fromId());
                BInputMessage newMessage(message.id(), message.fromId(), gid, message.date(),
                                         message.message(), message.fwdFromId(), message.fwdDate(), message.replyToMsgId(),
                                         message.media().classType(), accessLevel);
                eventNewMessage(newMessage);
            }
        }
        else if (update.classType() == Update::typeUpdateChatParticipants)
            eatChatParticipantsData(update.participants());
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

    onUpdates(updates, users, chats, date, seq);

    qCDebug(BOT_CORE) << ", seqStart=" << seqStart << ", seq=" << seq;
}

void Bot::onUpdateShort(Update update, qint32 date)
{
    qCDebug(BOT_CORE) << "Update Short: class=" << updateCode(update.classType()) << ", date=" << date;
    if (update.classType() == Update::typeUpdateUserStatus)
        qCDebug(BOT_CORE) << "Details: user=" << update.userId() << ", username=" << update.username() << ", status=" <<
                  ((update.status().classType() == UserStatus::typeUserStatusOnline) ? "Online" :
                  ((update.status().classType() == UserStatus::typeUserStatusOffline) ? "Offline" : "Unknown"))
               << ", wasOnline=" << update.status().wasOnline();
    else if (update.classType() == Update::typeUpdateUserName)
        eatUserDataChange(update.userId(), update.firstName(), update.lastName(), update.username());
}

void Bot::onUpdateShortMessage(qint32 id, qint32 userid, const QString &message, qint32 pts,
                               qint32 pts_count, qint32 date, qint32 fwd_from_id, qint32 fwd_date,
                               qint32 reply_to_msg_id, bool unread, bool out)
{
    Q_UNUSED(pts)
    Q_UNUSED(pts_count)
    Q_UNUSED(unread)
    Q_UNUSED(out)

    BInputMessage newMessage(id, userid, 0, date, message, fwd_from_id, fwd_date, reply_to_msg_id,
                             MessageMedia::typeMessageMediaEmpty, userAccessLevel(0, userid));
    eventNewMessage(newMessage);
}

void Bot::onUpdateShortChatMessage(qint32 id, qint32 fromId, qint32 chatId, const QString &message,
                                   qint32 pts, qint32 pts_count, qint32 date, qint32 fwd_from_id,
                                   qint32 fwd_date, qint32 reply_to_msg_id, bool unread, bool out)
{
    Q_UNUSED(pts)
    Q_UNUSED(pts_count)
    Q_UNUSED(unread)
    Q_UNUSED(out)

    BInputMessage newMessage(id, fromId, chatId, date, message, fwd_from_id, fwd_date, reply_to_msg_id,
                             MessageMedia::typeMessageMediaEmpty, userAccessLevel(chatId, fromId));
    eventNewMessage(newMessage);
}

void Bot::onUpdatesTooLong()
{
    qCWarning(BOT_CORE) << "Updates too long :|" << endl << flush;
}

QString Bot::updateCode(int code)
{
    switch (code){
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
    if (message.replyFromId() != 0 && message.replyFromUser() == 0)
    {
        resolveReplyFrom(message);
        return;
    }

    qCDebug(BOT_CORE) << "NewMessage Event: id= " << message.id() << ", user=" << message.userId() << ", chat=" << message.chatId() <<
              ", message=" << message.message() << ", date=" << message.date().toString() << ", fwdFrom=" << message.forwardedFrom() << ", fwdDate=" <<
              message.forwardedDate().toString() << "Reply From: " << message.replyFromId() << ", " << message.replyFromUser() <<
              ", " << message.replyFromMessage() << ", media: " << message.messageMediaType();

    foreach (Module *module, mModules)
        module->onNewMessage(message);
}

void Bot::eventCreateChat(qint64 gid, qint64 creator)
{
    qCDebug(BOT_CORE) << "Created Chat: " << gid << " By " << creator;
}

void Bot::eventAddUser(qint64 gid, qint64 user, qint64 inviter)
{
    qCDebug(BOT_CORE) << "Added user: " << user << " By " << inviter << " In " << gid;
}

void Bot::eventDelUser(qint64 gid, qint64 user, qint64 remover)
{
    qCDebug(BOT_CORE) << "Removed user: " << user << " By " << remover << " In " << gid;
    eatDelUser(gid, user);
}

void Bot::eventChangeTitle(qint64 gid, const QString &title, qint64 by)
{
    qCDebug(BOT_CORE) << "Change title: " << gid << " By " << by << " to " << title;
    eatChangeTitle(gid, title);
}

//End Events

//Start Module System

void Bot::installModule(Module *module)
{
    module->setBotInterface(mBotInterface);
    mCoreDataStore->updateModuleInfo(module);
    mModules.append(module);
}

void Bot::init()
{
    foreach (Module *module, mModules)
        module->init();

    foreach (Module *module, mModules)
        module->internalInit();

    mTelegram->init();
}

//End Module System

//Cron
void Bot::startCron()
{
    auto currentTime = QTime::currentTime();
    auto nextMinute = currentTime.addSecs(60);
    auto targetTime = QTime(nextMinute.hour(), nextMinute.minute());
    auto msecsToWait = qMax(currentTime.msecsTo(targetTime), 0);
    mTimer->start(msecsToWait + 1);
}

void Bot::cronTask()
{
    qCInfo(BOT_CORE) << tr("Running Cron (%1)").arg(QDateTime::currentDateTime().toString());

    qCInfo(BOT_CORE) << tr("Sending Keep-Alive...");
    mTelegram->wake();
    mTelegram->accountUpdateStatus(false);

    mTimer->start(60000);
}

//About
QString Bot::aboutText() const
{
    auto result = tr("Telegram Bot - Version: %1 (%2)\n").arg(version())
            .arg(versionDate().toString(BotUtils::DATE_FORMAT));

    result += tr("This is Telegram Bot! A multi-purpose chatbot.\n"
                 "My first version released on 2014/10/28 and "
                 "worked in some academic groups for several months.\n"
                 "Then I got completely rewritten to become a better bot "
                 "and my first general version has been released on 2015/09/11\n"
                 "More info at: http://telegram-bot.org/");

    return result;
}
