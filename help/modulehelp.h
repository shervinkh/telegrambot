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
    QList<QString> mNotes;

public:
    ModuleHelp();
    ModuleHelp(const QString &description);

    void addUsage(const ModuleHelpUsage &usage) { mUsages.append(usage); }
    void addNote(const QString &note) { mNotes.append(note); }

    QString description() const { return mDescription; }
    QList<ModuleHelpUsage> usages() const { return mUsages; }
    QList<QString> notes() const { return mNotes; }
};

#endif // MODULEHELP_H
