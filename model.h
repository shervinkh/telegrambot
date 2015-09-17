#ifndef MODEL_H
#define MODEL_H

#include "module.h"
#include "botinterface.h"

#include <QObject>
#include <QDate>
#include <QMetaProperty>
#include <QSharedPointer>

template<typename ModelClass>
class ObjectSet
{
private:
    QString mWhereClause;
    QList<QVariant> mWhereClauseValues;
    QString mOrderByClause;
    QString mLimitClause;

    ModelClass *mModel;
    BotInterface *mBotInterface;

    QString conditionString() const
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

public:
    ObjectSet(ModelClass *model, BotInterface *botInterface)
        : mModel(model), mBotInterface(botInterface) {}

    template<typename... Types>
    ObjectSet<ModelClass> filter(const QString &whereClause, Types... args)
    {
        ObjectSet<ModelClass> newObjectSet(*this);

        if (!newObjectSet.mWhereClause.isEmpty())
            newObjectSet.mWhereClause += " AND ";
        newObjectSet.mWhereClause += QString("(%1)").arg(whereClause);
        newObjectSet.mWhereClauseValues.append(BotUtils::convertArgsToList(args...));

        return newObjectSet;
    }

    template<typename... Types>
    ObjectSet<ModelClass> orderby(Types... args)
    {
        ObjectSet<ModelClass> newObjectSet(*this);

        QList<QVariant> argsList = BotUtils::convertArgsToList(args...);
        foreach (QVariant arg, argsList)
        {
            bool desc = arg.toString().startsWith('-');
            QString name = desc ? arg.toString().mid(1) : arg.toString();

            if (!newObjectSet.mOrderByClause.isEmpty())
                newObjectSet.mOrderByClause += ", ";

            newObjectSet.mOrderByClause += QString("%1 %2").arg(name).arg(desc ? "DESC" : "ASC");
        }

        return newObjectSet;
    }

    ObjectSet<ModelClass> limit(int start, int count)
    {
        ObjectSet<ModelClass> newObjectSet(*this);

        newObjectSet.mLimitClause = QString("LIMIT %1 OFFSET %2").arg(count).arg(start);

        return newObjectSet;
    }

    ObjectSet<ModelClass> limit(int count)
    {
        return limit(0, count);
    }

    QList<typename ModelClass::PointerType> select()
    {
        if (mOrderByClause.isEmpty())
            return orderby("id").select();

        QStringList fields = mModel->fields();

        QList<typename ModelClass::PointerType> result;

        if (mBotInterface)
        {
            QString queryString = QString("SELECT %1 FROM %2%3").arg(fields.join(", "))
                    .arg(mBotInterface->getModelDatabaseTable(mModel)).arg(conditionString());

            QSqlQuery query;
            if (mWhereClauseValues.isEmpty())
                query = mBotInterface->executeDatabaseQuery(queryString);
            else
            {
                query.prepare(queryString);
                foreach (QVariant value, mWhereClauseValues)
                    query.addBindValue(value);

                mBotInterface->executeDatabaseQuery(query);
            }

            while (query.next())
            {
                typename ModelClass::PointerType modelObject = mModel->newInstance();

                for (int i = 0; i < fields.size(); i++)
                    modelObject->setProperty(fields[i].toLocal8Bit().data(), query.value(i));

                result.append(modelObject);
            }
        }

        return result;
    }

    int deleteObjects()
    {
        if (!mOrderByClause.isEmpty() || !mLimitClause.isEmpty())
        {
            qDebug() << "Ignoring delete objects with order_by and/or limit clause";
            return 0;
        }

        if (mBotInterface)
        {
            QString queryString = QString("DELETE FROM %1%2")
                    .arg(mBotInterface->getModelDatabaseTable(mModel)).arg(conditionString());

            QSqlQuery query;
            if (mWhereClauseValues.isEmpty())
                query = mBotInterface->executeDatabaseQuery(queryString);
            else
            {
                query.prepare(queryString);
                foreach (QVariant value, mWhereClauseValues)
                    query.addBindValue(value);

                mBotInterface->executeDatabaseQuery(query);
            }

            return BotUtils::getNumRowsAffected(query);
        }

        return 0;
    }

    template<typename... Types>
    int update(const QString &updateClause, Types... args)
    {
        QList<QVariant> updateClauseValues = BotUtils::convertArgsToList(args...);

        if (mBotInterface)
        {
            QString queryString = QString("UPDATE %1 SET %2%3")
                    .arg(mBotInterface->getModelDatabaseTable(mModel)).arg(updateClause).arg(conditionString());

            QSqlQuery query;
            if (mWhereClauseValues.isEmpty() && updateClauseValues.isEmpty())
                query = mBotInterface->executeDatabaseQuery(queryString);
            else
            {
                query.prepare(queryString);
                foreach (QVariant value, updateClauseValues)
                    query.addBindValue(value);
                foreach (QVariant value, mWhereClauseValues)
                    query.addBindValue(value);

                mBotInterface->executeDatabaseQuery(query);
            }

            return BotUtils::getNumRowsAffected(query);
        }

        return 0;
    }
};

#define DECLARE_MODEL(klass, m_name, ver, ver_d) \
public: \
    Q_INVOKABLE QString name() const { return #m_name; } \
    Q_INVOKABLE qint64 version() const { return ver; } \
    Q_INVOKABLE QDate versionDate() const { return ver_d; } \
    Q_INVOKABLE QString section() const { return mSection; } \
    bool save() \
    { \
        if (mBotInterface) \
            return mBotInterface->saveModelObject(this); \
        return false; \
    } \
    bool deleteObject() \
    { \
        if (mBotInterface) \
            return mBotInterface->deleteModelObject(this); \
        return false; \
    } \
    typedef QSharedPointer<klass> PointerType; \
    PointerType newInstance() \
    { \
        klass *newObject = new klass(); \
        newObject->setup(mBotInterface, mSection); \
        return PointerType(newObject); \
    } \
    Q_INVOKABLE QStringList fields() \
    { \
        QStringList modelFields; \
        for (int i = 1; i < metaObject()->propertyCount(); i++) \
            modelFields.append(metaObject()->property(i).name()); \
        return modelFields; \
    } \
    ObjectSet<klass> objectSet() \
    { \
        return ObjectSet<klass>(sharedInstance(), mBotInterface); \
    } \
    static klass *sharedInstance() \
    { \
        if (!mSharedInstance) \
            mSharedInstance = new klass(); \
        return mSharedInstance; \
    } \
    Q_INVOKABLE void setup(BotInterface *botInterface, const QString &section) \
    { \
        mBotInterface = botInterface; \
        mSection = section; \
    } \
private: \
    klass() : QObject(Q_NULLPTR), mBotInterface(Q_NULLPTR), m_id(-1) {} \
    static klass *mSharedInstance; \
    BotInterface *mBotInterface; \
    QString mSection; \
    DECLARE_MODEL_FIELD(qint64, id)

#define DEFINE_MODEL(name) name *name::mSharedInstance = Q_NULLPTR;

#define DECLARE_MODEL_FIELD(type, member) \
public: \
    void set_##member(type _##member) \
    { \
        m_##member = _##member;\
    } \
    type get_##member() const\
    { \
        return m_##member;\
    } \
private: \
    type m_##member; \
    Q_PROPERTY(type member READ get_##member WRITE set_##member)

#define MODEL(klass) klass::sharedInstance()

#endif // MODEL_H
