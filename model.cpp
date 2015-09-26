#include "botinterface.h"
#include "model.h"

#include <QRegularExpression>

Model::Model(BotInterface *botInterface, const QString &section, const QString &name,
             qint64 version, const QDate &versionDate)
    : mBotInterface(botInterface), mSection(section), mName(name), mVersion(version),
      mVersionDate(versionDate)
{
    auto assertWhere = QString("Creating model \"%1\"")
            .arg(fullName()).toLocal8Bit();
    auto assertWhat = QByteArray("Invalid model name. Model name can only contain "
                                 "lowercase english characters and underscore.");
    Q_ASSERT_X(!fullName().contains(QRegularExpression("[^a-z_]")),
               assertWhere.data(), assertWhat.data());

    addField("id", ModelField::Integer);
}

ModelField &Model::addField(const QString &fieldName, ModelField::FieldType fieldType)
{
    auto assertWhere = QString("Adding field \"%1\" to model \"%2\"")
            .arg(fieldName).arg(fullName()).toLocal8Bit();
    auto assertWhatInvalidName = QByteArray("Invalid field name. Field name can only contain "
                                            "lowercase english characters and underscore.");
    auto assertWhatAlreadyExists = QByteArray("Field with the same name already exists.");

    Q_ASSERT_X(!fieldName.contains(QRegularExpression("[^a-z_]")),
               assertWhere.data(), assertWhatInvalidName.data());

    Q_ASSERT_X(!mFields.contains(fieldName), assertWhere.data(),
               assertWhatAlreadyExists.data());

    auto newField = ModelField(fieldType);
    mFields[fieldName] = newField;
    return mFields[fieldName];
}
