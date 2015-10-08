#ifndef HELP_H
#define HELP_H

#include "module.h"

class Help : public Module
{
private:
    QString helpString();

protected:
    ModuleHelp help() const;

public:
    Help();
    void init();
    void onNewMessage(BInputMessage message);
};

#endif // HELP_H
