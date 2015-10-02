#ifndef CONFIGFIELD_H
#define CONFIGFIELD_H

#include <QVariant>

class ConfigField
{
public:
    enum FieldType {Integer, String, Boolean};

private:
    friend class BotConfig;

    FieldType mFieldType;
    bool mIsGlobal;
    QVariant mValue;
    QVariant mDefaultValue;

public:
    ConfigField();
    ConfigField(FieldType fieldType, bool isGlobal, QVariant defaultValue);

    void restoreDefault() { mValue = mDefaultValue; }
    bool isGlobal() const { return mIsGlobal; }
    QVariant value() const { return mValue; }
    bool canTakeValue(const QVariant &value) const;
    QString typeString() const;
};

#endif // CONFIGFIELD_H
