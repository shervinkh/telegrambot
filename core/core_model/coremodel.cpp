#include "coremodel.h"
#include "model.h"
#include "botinterface.h"

#include <QTimeZone>

CoreModel::CoreModel(BotInterface *botInterface, QObject *parent)
    : QObject(parent), mBotInterface(botInterface)
{
    ensureDatabase();
}

void CoreModel::registerModels()
{
    auto installedModelModel = mBotInterface->newModel("core", "installed_model", 0, QDate(2015, 9, 17));
    installedModelModel->addField("section", ModelField::String);
    installedModelModel->addField("name", ModelField::String);
    installedModelModel->addField("installed_version", ModelField::Integer);
    installedModelModel->addField("version_date", ModelField::Timestamp);
    installedModelModel->addField("installed_date", ModelField::Timestamp);
    mBotInterface->registerModel(installedModelModel);
}

void CoreModel::init()
{
    registerModels();
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

void CoreModel::updateModelInfo(Model *model)
{
    auto modelsList = mBotInterface->model("core", "installed_model")->objectSet().
            filter("section=? AND name=?", model->section(), model->name()).select();
    auto thisModel = modelsList.isEmpty() ?
                mBotInterface->model("core", "installed_model")->newObject() : modelsList.first();

    if (thisModel->id() != -1 && thisModel["installed_version"] == model->version())
        return;

    if (thisModel->id() == -1)
    {
        thisModel["section"] = model->section();
        thisModel["name"] = model->name();
    }

    thisModel["installed_version"] = model->version();
    thisModel["version_date"] = QDateTime(model->versionDate());
    thisModel["installed_date"] = QDateTime::currentDateTime();

    thisModel->save();
}

void CoreModel::registerModel(Model *model)
{
    auto assertWhere = QString("Registering new model \"%1\"").arg(model->fullName()).toLocal8Bit();
    auto assertWhat = QByteArray("Model with the same name already exists");

    Q_ASSERT_X(!mRegisteredModels.contains(model->fullName()), assertWhere.data(), assertWhat.data());

    mRegisteredModels[model->fullName()] = model;
    updateModelInfo(model);
}

Model *CoreModel::model(const QString &section, const QString &name)
{
    auto fullName = QString("%1_%2").arg(section).arg(name);
    auto assertWhere = QString("Getting model \"%1\"").arg(fullName).toLocal8Bit();
    auto assertWhat = QByteArray("No such model exists!");

    Q_ASSERT_X(mRegisteredModels.contains(fullName), assertWhere.data(), assertWhat.data());

    return mRegisteredModels[fullName];
}
