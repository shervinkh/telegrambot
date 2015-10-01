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

QString Model::databaseTable() const
{
    if (!mOverridedDatabaseTable.isEmpty())
        return QString("bot_%1").arg(mOverridedDatabaseTable);
    else
        return QString("bot_%1s").arg(fullName());
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

void Model::addIndexImp(const QList<QVariant> &args, bool unique)
{
    auto indexType = unique ? "unique index" : "index";

    QString name;
    QStringList columns;
    foreach (auto column, args)
    {
        columns.append(column.toString());

        if (!name.isEmpty())
            name += "_";
        name += column.toString();
    }

    name.append("_index");
    if (unique)
        name.prepend("unique_");

    name.prepend(databaseTable() + "_");

    auto assertWhere = QString("Adding %1 \"%2\" to model \"%3\"").arg(indexType)
            .arg(name).arg(fullName()).toLocal8Bit();
    auto assertWhatInvalidName = QByteArray("Invalid index name. Field name can only contain "
                                            "lowercase english characters and underscore.");
    auto assertWhatAlreadyExists = QByteArray("Index with the same name already exists.");
    auto assertWhatEmptyList = QByteArray("No columns specified.");
    auto assertWhatNoColumn = QByteArray("No such column exists.");

    Q_ASSERT_X(!name.contains(QRegularExpression("[^a-z_]")),
               assertWhere.data(), assertWhatInvalidName.data());

    Q_ASSERT_X((!mIndexes.contains(name) && !mUniqueIndexes.contains(name)),
               assertWhere.data(), assertWhatAlreadyExists.data());

    Q_ASSERT_X(!args.isEmpty(), assertWhere.data(), assertWhatEmptyList.data());


    foreach (auto column, columns)
        Q_ASSERT_X(mFields.contains(column), assertWhere.data(), assertWhatNoColumn.data());

    if (unique)
        mUniqueIndexes[name] = columns;
    else
        mIndexes[name] = columns;
}

QString Model::createTableQuery() const
{
    auto query = QString("CREATE TABLE IF NOT EXISTS %1 (\n"
                         "id bigserial PRIMARY KEY").arg(databaseTable());

    foreach (auto field, fields())
        if (field != "id")
        {
            auto modelField = mFields[field];
            auto column = QString("%1 %2").arg(field)
                    .arg(ModelField::databaseTypeForFieldType(modelField.mFieldType));

            if (modelField.mNotNull)
                column += " NOT NULL";

            if (!modelField.mForeignKeyTable.isEmpty())
                column += QString(" REFERENCES %1(id) ON DELETE CASCADE ON UPDATE CASCADE")
                        .arg(modelField.mForeignKeyTable);

            query += ",\n" + column;
        }

    query += "\n)";

    return query;
}

QList<QString> Model::createIndexQueries() const
{
    QList<QString> queries;

    foreach (auto index, mIndexes.keys())
    {
        auto query = QString("CREATE INDEX %1 ON %2(%3)").arg(index)
                .arg(databaseTable()).arg(mIndexes[index].join(", "));
        queries.append(query);
    }

    foreach (auto uniqueIndex, mUniqueIndexes.keys())
    {
        auto query = QString("CREATE UNIQUE INDEX %1 ON %2(%3)").arg(uniqueIndex)
                .arg(databaseTable()).arg(mUniqueIndexes[uniqueIndex].join(", "));
        queries.append(query);
    }

    return queries;
}

QList<QString> Model::createQueries() const
{
    QList<QString> queries;
    queries.append(createTableQuery());
    queries.append(createIndexQueries());
    return queries;
}
