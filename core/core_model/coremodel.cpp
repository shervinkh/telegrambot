#include "coremodel.h"
#include "model.h"
#include "botinterface.h"

#include <QTimeZone>

CoreModel::CoreModel(BotInterface *botInterface, QObject *parent)
    : QObject(parent), mBotInterface(botInterface)
{
}

void CoreModel::registerModels()
{
    auto installedModelModel = mBotInterface->newModel("core", "installed_model", 0, QDate(2015, 9, 17));
    installedModelModel->addField("section", ModelField::String).notNull();
    installedModelModel->addField("name", ModelField::String).notNull();
    installedModelModel->addField("installed_version", ModelField::Integer).notNull();
    installedModelModel->addField("version_date", ModelField::Timestamp);
    installedModelModel->addField("installed_date", ModelField::Timestamp);
    installedModelModel->addUniqueIndex("section", "name");
    mBotInterface->registerModel(installedModelModel);
}

void CoreModel::init()
{
    registerModels();
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

    auto createQueries = model->createQueries();
    foreach (auto query, createQueries)
        mBotInterface->executeDatabaseQuery(query);

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
