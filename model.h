#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QMap>
#include <QVariant>

#define DECLARE_MODEL(module, name) \
public: \
    static name *sharedInstance() \
    { \
        if (!mSharedInstance) \
            mSharedInstance = new name(); \
        return mSharedInstance; \
    } \
private: \
    name() : Model(#module, #name) {} \
    static name *mSharedInstance;

#define DEFINE_MODEL(module, name) name *name::mSharedInstance = Q_NULLPTR;

#define DECLARE_MODEL_FIELD(type, member, ...) \
public: \
    void set_##member(type _##member) \
    { \
        m_##member = _##member;\
    } \
    type get_##member() const\
    { \
        return m_##member;\
    } \
    Q_INVOKABLE Properties get_##member##_properties() const\
    { \
        return makeProperties(__VA_ARGS__); \
    } \
private: \
    type m_##member; \
    Q_PROPERTY(type member READ get_##member WRITE set_##member)

class Model : public QObject
{
    Q_OBJECT
private:
    const QString mModuleName;
    const QString mName;

    typedef QMap<QString, QVariant> Properties;

    static Properties makeProperties();

    template<typename... Types>
    static Properties makeProperties(QString key, QVariant value, Types... rest)
    {
        Properties result = makeProperties(rest...);
        result[key] = value;
        return result;
    }

public:
    Model(const QString &moduleName, const QString &name);
    virtual ~Model() = 0;

    QMap<QString, QVariant> getFieldProperties(const QString &field);
};

#endif // MODEL_H
