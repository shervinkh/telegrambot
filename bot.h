#ifndef BOT_H
#define BOT_H

#include "binputmessage.h"
#include "bot.h"

#include <telegram.h>

#include <QObject>
#include <QMap>
#include <QLoggingCategory>
#include <QSqlQuery>

class Database;
class CoreDataStore;
class CoreModel;
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
    qint64 mMetadataStart = -1;
    QList<qint64> mLinkdataList;
    void updateMetadata();
    void updateUserGroupLinks();
    void updateNextGroupLinks();

    //DataGetter
    void getUserData(const User &user);
    void getChatData(const Chat &chat);
    void getChatParticipantsData(const ChatParticipants &chatParticipants);

    //Reply From System
    typedef QList<BInputMessage> MessageList;
    QMap<qint64, MessageList> pendingForReplyFromQueue;
    void resolveReplyFrom(BInputMessage message);

    //Events
    void eventNewMessage(BInputMessage message);

    //Modules
    QList<Module *> mModules;

    //Database
    Database *mDatabase;

    //Core
    CoreDataStore *mCoreDataStore;
    CoreModel *mCoreModel;

    //Redis
    Redis *mCoreRedis;
    Redis *mMetaRedis;

    //Timer
    QTimer *mTimer;

public:
    explicit Bot(Database *database, QObject *parent = 0);
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
    void onAuthCheckPhoneAnswer(qint64 id, bool phoneRegistered);
    void onAuthSendCodeAnswer(qint64 id, bool phoneRegistered, qint32 sendCallTimeout);
    void onAuthSignInError(qint64 id, qint32 errorCode, const QString &errorText);
    void onAuthLoggedIn();

    //Messages
    void onMessagesGetMessagesAnswer(qint64 id, qint32 sliceCount, const QList<Message> &messages,
                                   const QList<Chat> &chats, const QList<User> &users);
    void onMessagesGetFullChatAnswer(qint64 id, const ChatFull &chatFull, const QList<Chat> &chats, const QList<User> &users);
    void onMessagesGetDialogsAnswer(qint64 id, qint32 sliceCount, const QList<Dialog> &dialogs,
                                    const QList<Message> &messages, const QList<Chat> &chats, const QList<User> &users);

    //Error
    void onError(qint64 id, qint32 errorCode, QString errorText);

    //Updates
    void onUpdates(QList<Update> updates, QList<User> users, QList<Chat> chats, qint32 date, qint32 seq);
    void onUpdatesCombined(QList<Update> updates, QList<User> users, QList<Chat> chats, qint32 date, qint32 seqStart, qint32 seq);
    void onUpdateShort(Update update, qint32 date);
    void onUpdateShortMessage(qint32 id, qint32 userid, const QString &message, qint32 pts, qint32 pts_count,
                            qint32 date, qint32 fwd_from_id, qint32 fwd_date, qint32 reply_to_msg_id,
                            bool unread, bool out);
    void onUpdateShortChatMessage(qint32 id, qint32 fromId, qint32 chatId, const QString &message, qint32 pts,
                                qint32 pts_count, qint32 date, qint32 fwd_from_id, qint32 fwd_date,
                                qint32 reply_to_msg_id, bool unread, bool out);
    void onUpdatesTooLong();

    //Cron Task
    void cronTask();

    //Implementations
    void updateNextGroupLinksImp();
};

#endif // BOT_H
