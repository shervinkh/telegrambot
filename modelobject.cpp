#include "modelobject.h"
#include "botinterface.h"
#include "model.h"

ModelObject::ModelObject(Model *model)
    : mModel(model)
{
    foreach (auto field, model->fields())
        mData[field] = QVariant();
}

void ModelObject::assertFieldExists(const QString &fieldName) const
{
    auto assertWhere = QString("Getting field \"%1\" from model \"%2\"")
            .arg(fieldName).arg(mModel->fullName()).toLocal8Bit();

    auto assertWhat = "No such field exists!";

    Q_ASSERT_X(mData.contains(fieldName), assertWhere.data(), assertWhat);
}

const QVariant &ModelObject::operator[](const QString &fieldName) const
{
    assertFieldExists(fieldName);
    return mData[fieldName];
}

QVariant &ModelObject::operator[](const QString &fieldName)
{
    assertFieldExists(fieldName);
    return mData[fieldName];
}

bool ModelObject::save()
{
    QSqlQuery query;
    QStringList fields;
    QStringList questionMarks;
    QList<QVariant> values;

    foreach (auto field, mModel->fields())
        if (field != "id")
        {
            fields.append(field);
            values.append(mData[field]);
            questionMarks.append("?");
        }

    if (id() == -1)
    {
        auto fieldString = fields.join(", ");
        auto questionMarkString = questionMarks.join(", ");

        query.prepare(QString("INSERT INTO %1(%2) VALUES(%3)")
                      .arg(mModel->databaseTable()).arg(fieldString).arg(questionMarkString));
        foreach (auto value, values)
            query.addBindValue(value);

        mModel->mBotInterface->executeDatabaseQuery(query);

        mData["id"] = query.lastInsertId().toLongLong();

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

        query.prepare(QString("UPDATE %1 SET %2 WHERE id=?").arg(mModel->databaseTable()).arg(updateQuery));
        foreach (auto value, values)
            query.addBindValue(value);

        query.addBindValue(id());

        mModel->mBotInterface->executeDatabaseQuery(query);

        return BotUtils::getNumRowsAffected(query);
    }
}

bool ModelObject::deleteObject()
{
    QSqlQuery query;

    if (id() != -1)
    {
        query.prepare(QString("DELETE FROM %1 WHERE id=?").arg(mModel->databaseTable()));
        query.addBindValue(id());

        mModel->mBotInterface->executeDatabaseQuery(query);

        auto result = BotUtils::getNumRowsAffected(query);
        if (result)
            mData["id"] = -1;

        return result;
    }

    return false;
}

qint64 ModelObject::id()
{
    return mData["id"].isValid() ? mData["id"].toLongLong() : -1;
}
