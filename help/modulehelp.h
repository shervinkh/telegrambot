#ifndef MODULEHELP_H
#define MODULEHELP_H

#include "modulehelpusage.h"

#include <QObject>
#include <QList>

class ModuleHelp
{
private:
    QString mDescription;
    QList<ModuleHelpUsage> mUsages;

public:
    ModuleHelp();
    ModuleHelp(const QString &description);

    void addUsage(const ModuleHelpUsage &usage) { mUsages.append(usage); }

    QString description() const { return mDescription; }
    QList<ModuleHelpUsage> usages() const { return mUsages; }
};

#endif // MODULEHELP_H
