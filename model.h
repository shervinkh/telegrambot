#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QDate>
#include <QMap>

#include "modelobjectset.h"
#include "modelobject.h"
#include "modelobjectpointer.h"
#include "modelfield.h"

class BotInterface;

class Model
{
private:
    friend class ModelObjectSet;
    friend class ModelObject;

    BotInterface *mBotInterface;

    QString mSection;
    QString mName;
    qint64 mVersion;
    QDate mVersionDate;

    QMap<QString, ModelField> mFields;
    QMap<QString, QStringList> mIndexes;
    QMap<QString, QStringList> mUniqueIndexes;

    QString mOverridedDatabaseTable;

    void addIndexImp(const QList<QVariant> &args, bool unique = false);

    QString createTableQuery() const;
    QList<QString> createIndexQueries() const;

public:
    Model() {}
    Model(BotInterface *botInterface, const QString &section, const QString &name,
          qint64 version, const QDate &versionDate);
    ModelField &addField(const QString &fieldName, ModelField::FieldType fieldType);
    void overrideDatabaseTable(const QString &name) { mOverridedDatabaseTable = name; }
    ModelObjectPointer newObject() { return ModelObjectPointer(new ModelObject(this)); }
    ModelObjectSet objectSet() { return ModelObjectSet(this); }
    void registerModel();

    template<typename... Types>
    void addIndex(Types... args)
    {
        return addIndexImp(BotUtils::convertArgsToList(args...));
    }

    template<typename... Types>
    void addUniqueIndex(Types... args)
    {
        return addIndexImp(BotUtils::convertArgsToList(args...), true);
    }

    QList<QString> createQueries() const;
    QString section() const { return mSection; }
    QString name() const { return mName; }
    QString fullName() const { return QString("%1_%2").arg(mSection).arg(mName); }
    QStringList fields() const { return mFields.keys(); }
    QString databaseTable() const;
    qint64 version() const { return mVersion; }
    QDate versionDate() const { return mVersionDate; }
};

#endif // MODEL_H
