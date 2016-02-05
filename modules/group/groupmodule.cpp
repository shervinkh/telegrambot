#include "groupmodule.h"
#include "botinterface.h"

const QString GroupModule::ckUserGroup = "user#%1.group";

GroupModule::GroupModule()
    : Module("group", 0, QDate(2016, 2, 5))
{

}

void GroupModule::init()
{
    registerCommand("group");
}

ModuleHelp GroupModule::help() const
{
    ModuleHelp result("This module lets you to choose a group and use group-only functionalities in private.");

    result.addUsage(ModuleHelpUsage("View your groups",
                                    "!group"));

    result.addUsage(ModuleHelpUsage("Set group",
                                    "!group set group_id",
                                    "!group set 12345678"));

    result.addUsage(ModuleHelpUsage("Unset group",
                                    "!group unset"));

    return result;
}

void GroupModule::registerModels() {
    auto subscriptionModel = newModel("group", 0, QDate(2016, 2, 5));
    subscriptionModel->addField("gid", ModelField::Integer).notNull();
    subscriptionModel->addField("uid", ModelField::Integer).notNull();
    subscriptionModel->addUniqueIndex("uid");
    subscriptionModel->registerModel();
}

void GroupModule::onNewMessage(BInputMessage message)
{
    QString response;

    if (message.command() == "group")
    {
        if (!message.isPrivate())
        {
            response = "Group module is not usable in groups. Only in private!";
            interface()->sendMessage(message.chatId(), true, response, message.id());
            return;
        }

        auto args = message.getArgumentsArray();

        if (args.size() > 2 && args[1].toString().startsWith("set") && args[2].canConvert(QVariant::LongLong))
        {
            auto gid = args[2].toLongLong();
            response = fSet(message.userId(), gid);
            cInvalidateUserGroup(message.userId());
        }
        else if (args.size() > 1 && args[1].toString().startsWith("unset"))
        {
            response = fUnset(message.userId());
            cInvalidateUserGroup(message.userId());
        }
        else
        {
            response = fGroupList(message.userId());
        }
    }

    interface()->sendMessage(message.userId(), false, response, message.id());
}

QString GroupModule::fSet(qint64 uid, qint64 gid)
{
    auto groups = interface()->metadata()->userGroups(uid);
    if (!groups.contains(gid))
        return tr("You are not in that group!");

    fUnset(uid);

    auto newItem = model("group")->newObject();
    newItem["gid"] = gid;
    newItem["uid"] = uid;

    if (newItem->save())
        return tr("Group successfully set! Now you can use group-dependent functionalities in private.");
    else
        return tr("Falied to set group!");
}

QString GroupModule::fUnset(qint64 uid)
{
    if (model("group")->objectSet().filter("uid=?", uid).deleteObjects())
        return tr("Group successfully unset!");
    else
        return tr("Falied to unset! Maybe you have not set group yet.");
}

QString GroupModule::fGroupList(qint64 uid)
{
    QString response;

    auto groups = interface()->metadata()->userGroups(uid);
    if (groups.isEmpty())
        response = tr("We are not in any common group!");
    else
    {
        auto chosenGroup = cGetUserGroup(uid);
        for (int i = 0; i < groups.size(); i++)
        {
            auto groupMeta = interface()->metadata()->groupMetadata(groups[i]);
            auto star = (groupMeta.id() == chosenGroup) ? " (*)" : "";
            response += QString("%1- %2 (%3)%4").arg(i + 1).arg(groupMeta.id())
                    .arg(groupMeta.title()).arg(star);
            if (i != groups.size() - 1)
                response += "\n";
        }
    }

    return response;
}

qint64 GroupModule::cGetUserGroup(qint64 uid)
{
    auto result = redis()->getCachedValue(ckUserGroup.arg(uid), [this, uid] () -> QVariant {
                                              auto boards = model("group")->objectSet().filter("uid=?", uid).select();
                                              if (boards.isEmpty())
                                                  return -1;
                                              return boards.first()["gid"];
                                          });

    return result.toLongLong();
}

void GroupModule::cInvalidateUserGroup(qint64 uid)
{
    redis()->invalidateCache(ckUserGroup.arg(uid));
}

QVariant GroupModule::customCommand(const QString &command, const QList<QVariant> &args)
{
    auto assertWhere = QString("Custom command \"%1\" from module \"%2\"")
            .arg(command).arg(name()).toLocal8Bit();
    auto assertWhatInvalidArgs = "Invalid number of arguments!";
    auto assertWhatInvalidCommand = "Invalid command";

    if (command == "getUserGroup")
    {
        Q_ASSERT_X(args.size() == 1, assertWhere.data(), assertWhatInvalidArgs);
        auto uid = args[0].toLongLong();
        return cGetUserGroup(uid);
    }
    else
        Q_ASSERT_X(false, assertWhere.data(), assertWhatInvalidCommand);

    return QVariant();
}
