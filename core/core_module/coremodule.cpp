#include "coremodule.h"
#include "model.h"
#include "botinterface.h"
#include "module.h"

#include <QTimeZone>

CoreModule::CoreModule(BotInterface *botInterface, QObject *parent)
    : QObject(parent), mBotInterface(botInterface)
{
    registerModels();
}

void CoreModule::registerModels()
{
    auto installedModuleModel = mBotInterface->newModel("core", "installed_module", 0, QDate(2015, 9, 16));
    installedModuleModel->addField("name", ModelField::String).notNull();
    installedModuleModel->addField("installed_version", ModelField::Integer).notNull();
    installedModuleModel->addField("version_date", ModelField::Timestamp);
    installedModuleModel->addField("installed_date", ModelField::Timestamp);
    installedModuleModel->addUniqueIndex("name");
    installedModuleModel->registerModel();
}

void CoreModule::updateModuleInfo(Module *module)
{
    auto moduleInfos = mBotInterface->model("core", "installed_module")->objectSet()
            .filter("name=?", module->name()).select();

    auto moduleInfo = moduleInfos.isEmpty() ?
                mBotInterface->model("core", "installed_module")->newObject() : moduleInfos.first();

    if (moduleInfo->id() != -1 && moduleInfo["installed_version"].toLongLong() == module->version())
        return;

    if (moduleInfo->id() == -1)
        moduleInfo["name"] = module->name();

    moduleInfo["installed_version"] = module->version();
    moduleInfo["version_date"] = QDateTime(module->versionDate());
    moduleInfo["installed_date"] = QDateTime::currentDateTimeUtc();

    moduleInfo->save();
}
