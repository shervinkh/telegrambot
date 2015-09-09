#ifndef MODEL_H
#define MODEL_H

#include "module.h"

#include <QObject>
#include <QSharedPointer>

#define DECLARE_MODEL(klass, m_name) \
public: \
    Q_INVOKABLE QString name() const { return #m_name; } \
    void save() \
    { \
        if (mModule) \
            mModule->saveModelObject(this); \
    } \
    void deleteObject() \
    { \
        if (mModule) \
            mModule->deleteModelObject(this); \
    } \
    typedef QSharedPointer<klass> PointerType; \
    PointerType newInstance() \
    { \
        klass *newObject = new klass(); \
        newObject->setModule(mModule); \
        return QSharedPointer<klass>(newObject); \
    } \
    static klass *sharedInstance() \
    { \
        if (!mSharedInstance) \
            mSharedInstance = new klass(); \
        return mSharedInstance; \
    } \
    Q_INVOKABLE void setModule(Module *module) \
    { \
        mModule = module; \
    } \
private: \
    klass() : QObject(Q_NULLPTR), mModule(Q_NULLPTR), m_id(-1) {} \
    static klass *mSharedInstance; \
    Module *mModule;

#define DEFINE_MODEL(name) name *name::mSharedInstance = Q_NULLPTR;

#define DECLARE_MODEL_FIELD(type, member) \
public: \
    void set_##member(type _##member) \
    { \
        m_##member = _##member;\
    } \
    type get_##member() const\
    { \
        return m_##member;\
    } \
private: \
    type m_##member; \
    Q_PROPERTY(type member READ get_##member WRITE set_##member)

#define MODEL(klass) klass::sharedInstance()

#endif // MODEL_H
