#ifndef MODELOBJECTPOINTER_H
#define MODELOBJECTPOINTER_H

#include <QSharedData>

#include "modelobject.h"

class ModelObjectPointer : public QSharedPointer<ModelObject>
{
public:
    ModelObjectPointer();
    ModelObjectPointer(ModelObject *modelObject);

    const QVariant &operator[](const QString &name) const;
    QVariant &operator[](const QString &name);
};

#endif // MODELOBJECTPOINTER_H
