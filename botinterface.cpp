#include "botinterface.h"
#include "botutils.h"
#include "module.h"
#include "core/core_model/coremodel.h"
#include "database.h"
#include "redis.h"

#include <QMetaProperty>

BotInterface::BotInterface(Bot *bot, QObject *parent)
                           : QObject(parent), mBot(bot)
{

}

QString BotInterface::aboutText()
{
    return mBot->aboutText();
}

bool BotInterface::debug()
{
    return qgetenv("DEBUG") == "true";
}

Module *BotInterface::getModule(const QString &name)
{
    foreach (auto module, installedModules())
        if (module->name() == name)
            return module;

    return Q_NULLPTR;
}

GroupMetadata BotInterface::getGroupMetadata(qint64 gid)
{
    if (!mBot->mMetaRedis->exists(QString("chat#%1").arg(gid)).toBool())
        return GroupMetadata();

    auto title = mBot->mMetaRedis->hget(QString("chat#%1").arg(gid), "title").toString();
    auto adminId = mBot->mMetaRedis->hget(QString("chat#%1").arg(gid), "admin").toLongLong();

    return GroupMetadata(gid, title, adminId);
}

void BotInterface::executeDatabaseQuery(QSqlQuery &query)
{
    mBot->mDatabase->execute(query);
}

QSqlQuery BotInterface::executeDatabaseQuery(const QString &query)
{
    return mBot->mDatabase->execute(query);
}

InputPeer BotInterface::getPeer(qint64 id, bool chat)
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
        peer.setAccessHash(mBot->mMetaRedis->hget(QString("user#%1").arg(id), "access_hash").toLongLong());
    }

    return peer;
}

#include <QDebug>
void BotInterface::sendMessage(qint64 id, bool chat, const QString &message, qint64 replyTo)
{
    if (message.isEmpty())
        return;

    mBot->mTelegram->messagesSendMessage(getPeer(id, chat), BotUtils::secureRandomLong(), message, replyTo);
}

void BotInterface::sendBroadcast(const QList<qint64> &users, const QString &message)
{
    mBot->sendBroadcast(users, message);
}

void BotInterface::forwardMessage(qint64 id, bool chat, qint64 msgId)
{
    mBot->mTelegram->messagesForwardMessage(getPeer(id, chat), msgId);
}

void BotInterface::registerModel(const QString &section, QObject *model)
{
    model->metaObject()->invokeMethod(model, "setup", Q_ARG(BotInterface *, this), Q_ARG(QString, section));
    mBot->mCoreModel->updateModelInfo(model);
}

QString BotInterface::getModelDatabaseTable(QObject *object)
{
    QString modelSection, modelName;
    object->metaObject()->invokeMethod(object, "section", Q_RETURN_ARG(QString, modelSection));
    object->metaObject()->invokeMethod(object, "name", Q_RETURN_ARG(QString, modelName));
    return QString("bot_%1_%2s").arg(modelSection).arg(modelName);
}

int BotInterface::saveModelObject(QObject *object)
{
    qint64 pk = object->property("id").toLongLong();
    QSqlQuery query;

    QStringList fields;
    QList<QVariant> values;

    for (int i = 2; i < object->metaObject()->propertyCount(); i++)
    {
        fields.append(object->metaObject()->property(i).name());
        values.append(object->metaObject()->property(i).read(object));
    }

    if (pk == -1)
    {
        QString fieldString = fields.join(", ");
        QString questionMarks;
        QStringListIterator iter(fields);
        while (iter.hasNext())
        {
            iter.next();
            questionMarks += "?";
            if (iter.hasNext())
                questionMarks += ", ";
        }

        query.prepare(QString("INSERT INTO %1(%2) VALUES(%3)").arg(getModelDatabaseTable(object)).arg(fieldString).arg(questionMarks));
        foreach (QVariant value, values)
            query.addBindValue(value);

        executeDatabaseQuery(query);

        object->setProperty("id", query.lastInsertId().toLongLong());

        return BotUtils::getNumRowsAffected(query);
    }
    else
    {
        QString updateQuery;
        QStringListIterator iter(fields);
        while (iter.hasNext())
        {
            updateQuery += iter.next();
            updateQuery.append("=?");
            if (iter.hasNext())
                updateQuery += ", ";
        }

        query.prepare(QString("UPDATE %1 SET %2 WHERE id=?").arg(getModelDatabaseTable(object)).arg(updateQuery));
        foreach (QVariant value, values)
            query.addBindValue(value);

        query.addBindValue(object->property("id"));

        executeDatabaseQuery(query);

        return BotUtils::getNumRowsAffected(query);
    }
}

int BotInterface::deleteModelObject(QObject *object)
{
    qint64 pk = object->property("id").toLongLong();
    QSqlQuery query;

    if (pk != -1)
    {
        query.prepare(QString("DELETE FROM %1 WHERE id=?").arg(getModelDatabaseTable(object)));
        query.addBindValue(pk);

        executeDatabaseQuery(query);

        auto result = BotUtils::getNumRowsAffected(query);
        if (result)
            object->setProperty("id", -1);

        return result;
    }

    return 0;
}
