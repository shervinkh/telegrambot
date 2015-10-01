#include "modelfield.h"
#include "model.h"

ModelField::ModelField()
{

}

ModelField::ModelField(FieldType fieldType)
    : mFieldType(fieldType), mNotNull(false)
{

}

ModelField &ModelField::notNull()
{
    mNotNull = true;
    return *this;
}

ModelField &ModelField::foriegnKey(Model *model)
{
    mForeignKeyTable = model->databaseTable();
    return *this;
}

QString ModelField::databaseTypeForFieldType(FieldType fieldType)
{
    switch (fieldType)
    {
        case Integer:
            return "bigint";
        case String:
            return "text";
        case Boolean:
            return "boolean";
        case Timestamp:
            return "timestamp";
    }
}
