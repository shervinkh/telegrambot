#include "module.h"
#include "bot.h"
#include <QMetaProperty>

Module::Module(const QString name, const qint64 version, QObject *parent)
    : QObject(parent), mName(name.toLower()), mVersion(version),
      mLoggingCategoryName(QByteArray("bot.module.").append(mName)),
      mBot(Q_NULLPTR), mLoggingCategory(mLoggingCategoryName.data())
{

}

Module::~Module()
{

}

void Module::internalInit()
{
    ensureDatabase();
}

void Module::registerModel(QObject *model)
{
    model->metaObject()->invokeMethod(model, "setModule", Q_ARG(Module *, this));
}

QString Module::getModelDatabaseTable(QObject *object)
{
    QString modelName;
    object->metaObject()->invokeMethod(object, "name", Q_RETURN_ARG(QString, modelName));
    return QString("bot_modules_%1_%2s").arg(mName).arg(modelName);
}

void Module::saveModelObject(QObject *object)
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

        mBot->executeDatabaseQuery(query);

        object->setProperty("id", query.lastInsertId().toLongLong());
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

        mBot->executeDatabaseQuery(query);
    }
}

void Module::deleteModelObject(QObject *object)
{
    qint64 pk = object->property("id").toLongLong();
    QSqlQuery query;

    if (pk != -1)
    {
        query.prepare(QString("DELETE FROM %1 WHERE id=?").arg(getModelDatabaseTable(object)));
        query.addBindValue(pk);

        mBot->executeDatabaseQuery(query);
    }
}
