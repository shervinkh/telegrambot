#include "coremodel.h"
#include "installedmodelmodel.h"
#include <QTimeZone>

DEFINE_MODEL(InstalledModelModel)

CoreModel::CoreModel(BotInterface *botInterface, QObject *parent)
    : QObject(parent), mBotInterface(botInterface)
{
    ensureDatabase();
}

void CoreModel::init()
{
    mBotInterface->registerModel("core", MODEL(InstalledModelModel));
}

void CoreModel::ensureDatabase()
{
    mBotInterface->executeDatabaseQuery("CREATE TABLE IF NOT EXISTS bot_core_installed_models ("
                               "    id bigserial PRIMARY KEY,"
                               "    section text NOT NULL,"
                               "    name text NOT NULL,"
                               "    installed_version bigint NOT NULL,"
                               "    version_date timestamp with time zone,"
                               "    installed_date timestamp with time zone"
                               ")");

    mBotInterface->executeDatabaseQuery("CREATE UNIQUE INDEX unique_section_name_index "
                               "ON bot_core_installed_models(section, name)");
}

void CoreModel::updateModelInfo(QObject *model)
{
    QString section;
    QString name;
    qint64 version;
    QDate versionDate;

    model->metaObject()->invokeMethod(model, "section", Q_RETURN_ARG(QString, section));
    model->metaObject()->invokeMethod(model, "name", Q_RETURN_ARG(QString, name));
    model->metaObject()->invokeMethod(model, "version", Q_RETURN_ARG(qint64, version));
    model->metaObject()->invokeMethod(model, "versionDate", Q_RETURN_ARG(QDate, versionDate));

    auto modelsList = MODEL(InstalledModelModel)->objectSet().filter("section=? AND name=?", section, name).select();
    auto thisModel = modelsList.isEmpty() ? MODEL(InstalledModelModel)->newInstance() : modelsList.first();

    if (thisModel->property("id").toLongLong() != -1 && thisModel->property("installed_version") == version)
        return;

    if (thisModel->property("id").toLongLong() == -1)
    {
        thisModel->setProperty("section", section);
        thisModel->setProperty("name", name);
    }

    thisModel->setProperty("installed_version", version);
    thisModel->setProperty("version_date", QDateTime(versionDate));
    thisModel->setProperty("installed_date", QDateTime::currentDateTime());

    thisModel->save();
}
