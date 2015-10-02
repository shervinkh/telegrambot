#ifndef HELP_H
#define HELP_H

#include "module.h"

class Help : public Module
{
    DECLARE_MODULE(Help)
private:
    QString helpString();

protected:
    ModuleHelp help() const;

public:
    void init();
    void onNewMessage(BInputMessage message);
};

#endif // HELP_H
