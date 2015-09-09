#ifndef MODULE_H
#define MODULE_H

#include "binputmessage.h"
#include "botutils.h"

#include <QObject>
#include <QVariant>
#include <QSqlQuery>
#include <QLoggingCategory>

class Bot;

#define DECLARE_MODULE(name) \
public: \
    static name *instance() { \
        if (!mInstance) \
            mInstance = new name(); \
        return mInstance; \
    } \
private: \
    name(); \
    static name *mInstance;

#define DEFINE_MODULE(name) \
    name *name::mInstance = Q_NULLPTR;

#define MODULE(name) name::instance()

#define logDebug() qCDebug(mLoggingCategory)
#define logInfo() qCInfo(mLoggingCategory)
#define logWarning() qCWarning(mLoggingCategory)
#define logCritical() qCCritical(mLoggingCategory)

class Module : public QObject
{
    Q_OBJECT
private:
    const QString mName;
    const qint64 mVersion;
    const QByteArray mLoggingCategoryName;

protected:
    Bot *mBot;
    QLoggingCategory mLoggingCategory;

    virtual void ensureDatabase() {}
    void registerModel(QObject *model);

public:
    explicit Module(const QString name, const qint64 version, QObject *parent = Q_NULLPTR);
    virtual ~Module() = 0;

    void internalInit();
    void setBot(Bot *bot) { mBot = bot; }

    QString name() { return mName; }
    qint64 version() { return mVersion; }

    virtual void init() {}
    virtual void onNewMessage(BInputMessage message) { Q_UNUSED(message); }

    //Model Interface
    QString getModelDatabaseTable(QObject *object);
    void saveModelObject(QObject *object);
    void deleteModelObject(QObject *object);

public slots:

};

#endif // MODULE_H
