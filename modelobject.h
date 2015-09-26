#ifndef MODELOBJECT_H
#define MODELOBJECT_H

#include <QMap>
#include <QVariant>
#include <QSharedPointer>

class Model;

class ModelObject
{
private:
    Model *mModel;

    QMap<QString, QVariant> mData;

    void assertFieldExists(const QString &fieldName) const;

public:
    ModelObject(Model *model);

    const QVariant &operator[](const QString &fieldName) const;
    QVariant &operator[](const QString &fieldName);

    bool save();
    bool deleteObject();

    qint64 id();
};

#endif // MODELOBJECT_H
