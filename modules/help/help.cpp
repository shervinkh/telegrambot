#include "help.h"
#include "botinterface.h"

Help::Help()
    : Module("help", 0, QDate(2015, 9, 17))
{

}

void Help::init()
{
    registerCommand("help");
    registerCommand("about");
}

ModuleHelp Help::help() const
{
    ModuleHelp result("This module shows the manual of bot modules and bot's about text.");

    result.addUsage(ModuleHelpUsage("Module's help text",
                                    "!help, !help module_name",
                                    "!help help"));

    result.addUsage(ModuleHelpUsage("Bot's about text",
                                    "!about"));

    return result;
}

QString Help::helpString()
{
    auto aboutText = interface()->aboutText();
    aboutText += tr("\n\nSend \"!help module_name\" for more help on a specific module.");
    aboutText += tr("\nCurrently installed modules are:");

    foreach (auto module, interface()->installedModules())
        aboutText += QString(" %1").arg(module->name());

    return aboutText;
}

void Help::onNewMessage(BInputMessage message)
{
    QString response;
    auto inpm = message.isPrivate();

    if (message.command() == "help")
    {
        auto args = message.getArgumentsArray();

        if (args.size() > 1)
        {
            auto moduleName = args[1].toString();
            auto module = interface()->getModule(moduleName);

            if (module)
                response = module->helpString();
            else
                response = tr("No such module exists!");
        }
        else
            response = helpString();

    }
    else if (message.command() == "about")
        response = interface()->aboutText();

    interface()->sendMessage(inpm ? message.userId() : message.chatId(), !inpm, response, message.id());
}
