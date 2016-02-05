#ifndef MODULEHELPUSAGE_H
#define MODULEHELPUSAGE_H

#include <QObject>

class ModuleHelpUsage
{
private:
    QString mUsage;
    QString mUsageFormat;
    QString mUsageExample;

public:
    ModuleHelpUsage();
    ModuleHelpUsage(const QString &usage, const QString &usingFormat = QString(),
                    const QString &usageExample = QString());

    QString usage() const { return mUsage; }
    QString usageFormat() const { return mUsageFormat; }
    QString usageExample() const { return mUsageExample; }
};

#endif // MODULEHELPUSAGE_H
