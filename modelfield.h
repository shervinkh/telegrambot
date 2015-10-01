#ifndef MODELFIELD_H
#define MODELFIELD_H

#include <QString>
class Model;

class ModelField
{
public:
    enum FieldType {Integer, String, Boolean, Timestamp};

private:
    friend class Model;

    FieldType mFieldType;
    bool mNotNull;
    QString mForeignKeyTable;

public:
    ModelField();
    ModelField(FieldType fieldType);
    ModelField &notNull();
    ModelField &foriegnKey(Model *model);

    static QString databaseTypeForFieldType(FieldType fieldType);
};

#endif // MODELFIELD_H
