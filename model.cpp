#include "model.h"

QMap<QString, QVariant> Model::makeProperties()
{
    return QMap<QString, QVariant>();
}

Model::Model(const QString &moduleName, const QString &name)
    : mModuleName(moduleName), mName(name)
{

}

Model::~Model()
{

}

QMap<QString, QVariant> Model::getFieldProperties(const QString &field)
{
    Properties result;
    QByteArray methodName = QByteArray("get_").append(field.toLocal8Bit()).append("_properties");
    metaObject()->invokeMethod(this, methodName.data(), Q_RETURN_ARG(Properties, result));
    return result;
}
