#ifndef MODELFIELD_H
#define MODELFIELD_H


class ModelField
{
public:
    enum FieldType {Integer, String, Boolean, Timestamp};

private:
    FieldType mFieldType;

public:
    ModelField();
    ModelField(FieldType fieldType);
};

#endif // MODELFIELD_H
