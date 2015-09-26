#include "modelobjectset.h"
#include "botinterface.h"

ModelObjectSet::ModelObjectSet(Model *model)
    : mModel(model)
{

}

QString ModelObjectSet::conditionString() const
{
    QString condition;

    if (!mWhereClause.isEmpty())
        condition += QString(" WHERE %1").arg(mWhereClause);

    if (!mOrderByClause.isEmpty())
        condition += QString(" ORDER BY %1").arg(mOrderByClause);

    if (!mLimitClause.isEmpty())
        condition += QString(" %1").arg(mLimitClause);

    return condition;
}

ModelObjectSet ModelObjectSet::filterImp(const QString &whereClause, const QList<QVariant> &args)
{
    ModelObjectSet newObjectSet(*this);

    if (!newObjectSet.mWhereClause.isEmpty())
        newObjectSet.mWhereClause += " AND ";
    newObjectSet.mWhereClause += QString("(%1)").arg(whereClause);
    newObjectSet.mWhereClauseValues.append(args);

    return newObjectSet;
}

ModelObjectSet ModelObjectSet::orderbyImp(const QList<QVariant> &args)
{
    ModelObjectSet newObjectSet(*this);

    foreach (auto arg, args)
    {
        auto desc = arg.toString().startsWith('-');
        auto name = desc ? arg.toString().mid(1) : arg.toString();

        if (!newObjectSet.mOrderByClause.isEmpty())
            newObjectSet.mOrderByClause += ", ";

        newObjectSet.mOrderByClause += QString("%1 %2").arg(name).arg(desc ? "DESC" : "ASC");
    }

    return newObjectSet;
}

ModelObjectSet ModelObjectSet::limit(int start, int count)
{
    ModelObjectSet newObjectSet(*this);

    newObjectSet.mLimitClause = QString("LIMIT %1 OFFSET %2").arg(count).arg(start);

    return newObjectSet;
}

ModelObjectSet ModelObjectSet::limit(int count)
{
    return limit(0, count);
}

QList<ModelObjectPointer> ModelObjectSet::select()
{
    if (mOrderByClause.isEmpty())
        return orderby("id").select();

    auto fields = mModel->fields();

    QList<ModelObjectPointer> result;

    auto queryString = QString("SELECT %1 FROM %2%3").arg(fields.join(", "))
            .arg(mModel->databaseTable()).arg(conditionString());

    QSqlQuery query;
    if (mWhereClauseValues.isEmpty())
        query = mModel->mBotInterface->executeDatabaseQuery(queryString);
    else
    {
        query.prepare(queryString);
        foreach (auto value, mWhereClauseValues)
            query.addBindValue(value);

        mModel->mBotInterface->executeDatabaseQuery(query);
    }

    while (query.next())
    {
        auto modelObject = mModel->newObject();

        for (int i = 0; i < fields.size(); i++)
            modelObject[fields[i]] = query.value(i);

        result.append(modelObject);
    }

    return result;
}

int ModelObjectSet::deleteObjects()
{
    auto assertWhere = QString("Calling delete object on ObjectSet of model \"%1\"")
            .arg(mModel->fullName()).toLocal8Bit();
    auto assertWhat = QByteArray("order_by and/or limit clause are not supported in delete operation");

    Q_ASSERT_X((mOrderByClause.isEmpty() && mLimitClause.isEmpty()),
               assertWhere.data(), assertWhat.data());

    auto queryString = QString("DELETE FROM %1%2")
            .arg(mModel->databaseTable()).arg(conditionString());

    QSqlQuery query;
    if (mWhereClauseValues.isEmpty())
        query = mModel->mBotInterface->executeDatabaseQuery(queryString);
    else
    {
        query.prepare(queryString);
        foreach (auto value, mWhereClauseValues)
            query.addBindValue(value);

        mModel->mBotInterface->executeDatabaseQuery(query);
    }

    return BotUtils::getNumRowsAffected(query);
}

int ModelObjectSet::updateImp(const QString &updateClause, const QList<QVariant> &args)
{
    auto assertWhere = QString("Calling update on ObjectSet of model \"%1\"")
            .arg(mModel->fullName()).toLocal8Bit();
    auto assertWhat = QByteArray("order_by and/or limit clause are not supported in update operation");

    Q_ASSERT_X((mOrderByClause.isEmpty() && mLimitClause.isEmpty()),
               assertWhere.data(), assertWhat.data());

    auto queryString = QString("UPDATE %1 SET %2%3")
            .arg(mModel->databaseTable()).arg(updateClause).arg(conditionString());

    QSqlQuery query;
    if (mWhereClauseValues.isEmpty() && args.isEmpty())
        query = mModel->mBotInterface->executeDatabaseQuery(queryString);
    else
    {
        query.prepare(queryString);
        foreach (auto value, args)
            query.addBindValue(value);
        foreach (auto value, mWhereClauseValues)
            query.addBindValue(value);

        mModel->mBotInterface->executeDatabaseQuery(query);
    }

    return BotUtils::getNumRowsAffected(query);
}
