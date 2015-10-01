#include "subscribe.h"
#include "model.h"
#include "botinterface.h"

DEFINE_MODULE(Subscribe)

Subscribe::Subscribe()
    : Module("subscribe", 0, QDate(2015, 9, 18))
{

}

void Subscribe::init()
{
    registerCommand("subscribe");
    registerCommand("unsubscribe");
}

void Subscribe::registerModels()
{
    auto subscriptionModel = newModel("subscription", 0, QDate(2015, 9, 18));
    subscriptionModel->addField("gid", ModelField::Integer).notNull();
    subscriptionModel->addField("uid", ModelField::Integer).notNull();
    subscriptionModel->addField("subscribed_on", ModelField::Timestamp);
    subscriptionModel->addUniqueIndex("gid", "uid");
    subscriptionModel->addIndex("gid");
    registerModel(subscriptionModel);
}

ModuleHelp Subscribe::help() const
{
    ModuleHelp result("This module lets you subscribe to groups to be notified when an important event "
                      "happens. Like new entries in board module or new polls.");

    result.addUsage(ModuleHelpUsage("Subscribe", "!subscribe"));
    result.addUsage(ModuleHelpUsage("Unsubscribe", "!unsubscribe"));

    return result;
}

void Subscribe::onNewMessage(BInputMessage message)
{
    if (message.chatId())
    {
        QString response;

        if (message.command() == "subscribe")
            response = fSubscribe(message.chatId(), message.userId(), message.date());
        else if (message.command() == "unsubscribe")
            response = fUnsubscribe(message.chatId(), message.userId());

        interface()->sendMessage(message.chatId(), true, response, message.id());
    }
}

QString Subscribe::fSubscribe(qint64 gid, qint64 uid, const QDateTime &subscribed_on)
{
    auto newItem = model("subscription")->newObject();
    newItem["gid"] = gid;
    newItem["uid"] = uid;
    newItem["subscribed_on"] = subscribed_on;

    if (newItem->save())
        return tr("You subscribed successfully!");
    else
        return tr("Falied to subscribe! Maybe you have already subscribed.");
}

QString Subscribe::fUnsubscribe(qint64 gid, qint64 uid)
{
    if (model("subscription")->objectSet().filter("gid=? AND uid=?", gid, uid).deleteObjects())
        return tr("You unsubscribed successfully!");
    else
        return tr("Falied to unsubscribe! Maybe you have not subscribed.");
}

void Subscribe::customCommand(const QString &command, const QList<QVariant> &args)
{
    if (command == "sendNotification")
    {
        auto gid = args[0].toLongLong();
        auto tag = args[1].toString();
        auto text = args[2].toString();
        sendNotification(gid, tag, text);
    }
}

void Subscribe::sendNotification(qint64 gid, const QString &tag, const QString &text)
{
    auto groupMetadata = interface()->getGroupMetadata(gid);
    auto broadcastString = tr("Notification from %1 (%2):\n%3").arg(groupMetadata.title()).arg(tag).arg(text);

    auto subscriptions = model("subscription")->objectSet().filter("gid=?", gid).select();
    QList<qint64> users;
    foreach (auto subscription, subscriptions)
        users.append(subscription["uid"].toLongLong());

    interface()->sendBroadcast(users, broadcastString);
}
