#include "modelobjectpointer.h"

ModelObjectPointer::ModelObjectPointer()
    : QSharedPointer<ModelObject>()
{

}

ModelObjectPointer::ModelObjectPointer(ModelObject *modelObject)
    : QSharedPointer<ModelObject>(modelObject)
{

}

const QVariant &ModelObjectPointer::operator[](const QString &name) const
{
    return (*(*this))[name];
}

QVariant &ModelObjectPointer::operator[](const QString &name)
{
    return (*(*this))[name];
}
