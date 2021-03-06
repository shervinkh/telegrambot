#ifndef BOT_H
#define BOT_H

#include "binputmessage.h"
#include "bot.h"

#include <telegram.h>

#include <QObject>
#include <QMap>
#include <QLoggingCategory>
#include <QSqlQuery>
#include <QTimer>

class Database;
class CoreDataStore;
class CoreModel;
class CoreModule;
class CoreConfig;
class Module;
class BotInterface;
class Redis;

Q_DECLARE_LOGGING_CATEGORY(BOT_CORE)

class Bot : public QObject
{
    Q_OBJECT
private:
    static const QString sVersion;
    static const QDate sVersionDate;

    static const int METADATA_UPDATE_SLICE;
    friend class BotInterface;

    BotInterface *mBotInterface;

    Telegram *mTelegram;
    QTextStream input, output;

    enum Operation {None, AuthCheckPhone};
    Operation mCurrentOperation;

    bool mLoggedIn;

    QString updateCode(int code); //TODO: Delete it

    //MetaData
    qint64 mSuperuserId;
    qint64 mMetadataStart = -1;
    qint64 mMetadataDate = -1;
    InputPeer mMetadataPeer;
    QList<qint64> mLinkdataList;
    void updateMetadata();
    void updateNextMetadata();
    void updateUserGroupLinks();
    void updateNextGroupLinks();
    BInputMessage::AccessLevel userAccessLevel(qint64 gid, qint64 uid);

    //DataGetter
    void eatUserData(const User &user);
    void eatUserDataChange(qint64 id, const QString &firstName,
                           const QString &lastName, const QString &username);
    void eatChatData(const Chat &chat);
    void eatChatParticipantsData(const ChatParticipants &chatParticipants);
    void eatDelUser(qint64 gid, qint64 uid);
    void eatChangeTitle(qint64 gid, const QString &title);

    //Reply From System
    typedef QList<BInputMessage> MessageList;
    QMap<qint64, MessageList> pendingForReplyFromQueue;
    void resolveReplyFrom(BInputMessage message);
    void setPrivateGroup(BInputMessage &message);

    //Events
    void eventNewMessage(BInputMessage message);
    void eventCreateChat(qint64 gid, qint64 creator);
    void eventAddUser(qint64 gid, qint64 user, qint64 inviter);
    void eventDelUser(qint64 gid, qint64 user, qint64 remover);
    void eventChangeTitle(qint64 gid, const QString &title, qint64 by);

    //Modules
    QList<Module *> mModules;

    //Database
    Database *mDatabase;

    //Core
    CoreDataStore *mCoreDataStore;
    CoreModel *mCoreModel;
    CoreModule *mCoreModule;
    CoreConfig *mCoreConfig;

    //Redis
    Redis *mCoreRedis;
    Redis *mMetaRedis;

    //Timer
    void startCron();
    QTimer *mTimer;

    //Broadcast System
    QMap<qint64, QList<qint64>> mBroadcastUsers;
    void sendBroadcast(const QList<qint64> &users, const QString &message);
    void continueBroadcast(qint64 msgId, const QList<qint64> &users);

public:
    explicit Bot(Database *database, qint64 superuserId, QObject *parent = 0);
    void installModule(Module *module);
    void init();
    QString aboutText() const;
    Database *database() { return mDatabase; }
    BotInterface *interface() { return mBotInterface; }

    static QString version() { return sVersion; }
    static QDate versionDate() { return sVersionDate; }

public slots:
    //Auth
    void onAuthNeeded();
    void onAuthCheckPhoneAnswer(qint64 id, const AuthCheckedPhone &auth);
    void onAuthSendCodeAnswer(qint64 id, const AuthSentCode &auth);
    void onAuthSignInError(qint64 id, qint32 errorCode, const QString &errorText);
    void onAuthLoggedIn();

    //Messages
    void onMessagesGetMessagesAnswer(qint64 id, const MessagesMessages &messages);
    void onMessagesGetFullChatAnswer(qint64 id, const MessagesChatFull &chats);
    void onMessagesGetDialogsAnswer(qint64 id, const MessagesDialogs &dialogs);
    void onMessagesSendMessageAnswer(qint64 id, const UpdatesType &update);

    //Stated Messages
    QString decodeMessageAction(MessageAction state);

    //Error
    void onError(qint64 id, qint32 errorCode, QString errorText);
    void onMessagesGetFullChatError(qint64 id, qint32 errorCode, QString errorText);

    //Updates
    void onUpdates(QList<Update> updates, QList<User> users, QList<Chat> chats, qint32 date, qint32 seq);
    void onUpdatesCombined(QList<Update> updates, QList<User> users, QList<Chat> chats, qint32 date, qint32 seqStart, qint32 seq);
    void onUpdateShort(Update update, qint32 date);
    void onUpdateShortMessage(qint32 id, qint32 userid, const QString &message, qint32 pts, qint32 pts_count,
                            qint32 date, Peer fwd_from_id, qint32 fwd_date, qint32 reply_to_msg_id,
                            bool unread, bool out);
    void onUpdateShortChatMessage(qint32 id, qint32 fromId, qint32 chatId, const QString &message, qint32 pts,
                                qint32 pts_count, qint32 date, Peer fwd_from_id, qint32 fwd_date,
                                qint32 reply_to_msg_id, bool unread, bool out);
    void onUpdatesTooLong();

    //Cron Task
    void cronTask();

    //Implementations
    void updateNextGroupLinksImp();
    void updateNextMetadataImp();
};

#endif // BOT_H
