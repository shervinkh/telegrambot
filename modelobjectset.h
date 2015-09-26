#ifndef MODELOBJECTSET_H
#define MODELOBJECTSET_H

#include "modelobjectpointer.h"
#include "botutils.h"

class Model;

class ModelObjectSet
{
private:
    Model *mModel;

    QString mWhereClause;
    QList<QVariant> mWhereClauseValues;
    QString mOrderByClause;
    QString mLimitClause;

    QString conditionString() const;
    ModelObjectSet filterImp(const QString &whereClause, const QList<QVariant> &args);
    ModelObjectSet orderbyImp(const QList<QVariant> &args);
    int updateImp(const QString &updateClause, const QList<QVariant> &args);

public:
    ModelObjectSet(Model *model);

    template<typename... Types>
    ModelObjectSet filter(const QString &whereClause, Types... args)
    {
        return filterImp(whereClause, BotUtils::convertArgsToList(args...));
    }

    template<typename... Types>
    ModelObjectSet orderby(Types... args)
    {
        return orderbyImp(BotUtils::convertArgsToList(args...));
    }

    ModelObjectSet limit(int start, int count);
    ModelObjectSet limit(int count);
    QList<ModelObjectPointer> select();
    int deleteObjects();

    template<typename... Types>
    int update(const QString &updateClause, Types... args)
    {
        return updateImp(updateClause, BotUtils::convertArgsToList(args...));
    }
};

#endif // MODELOBJECTSET_H
