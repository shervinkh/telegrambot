#include "configfield.h"

ConfigField::ConfigField()
{

}

ConfigField::ConfigField(FieldType fieldType, bool isGlobal, QVariant defaultValue)
    : mFieldType(fieldType), mIsGlobal(isGlobal), mValue(defaultValue), mDefaultValue(defaultValue)
{

}

bool ConfigField::canTakeValue(const QVariant &value) const
{
    if (mFieldType == Integer)
        return value.canConvert(QVariant::Int);
    else if (mFieldType == Boolean)
    {
        auto strVal = value.toString().toLower().trimmed();
        auto trueOrFalse = (strVal == "true" || strVal == "false");
        return value.canConvert(QVariant::Bool) && trueOrFalse;
    }
    else if (mFieldType == String)
        return value.canConvert(QVariant::String);

    return false;
}

QString ConfigField::typeString() const
{
    switch (mFieldType)
    {
        case Integer:
            return "Integer";
        case String:
            return "String";
        case Boolean:
            return "Boolean";
    }
}
