#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T20:04:30
#
#-------------------------------------------------

QT       += core sql

QT       -= gui

TARGET = Telegram-Bot
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp \
    bot.cpp \
    signalhandler.cpp \
    module.cpp \
    binputmessage.cpp \
    database.cpp \
    botutils.cpp \
    redis.cpp \
    modules/board/board.cpp \
    botinterface.cpp \
    core/core_data_store/coredatastore.cpp \
    core/core_model/coremodel.cpp \
    modules/help/help.cpp \
    modules/subscribe/subscribe.cpp \
    model.cpp \
    modelfield.cpp \
    modelobject.cpp \
    modelobjectset.cpp \
    modelobjectpointer.cpp \
    botconfig.cpp \
    configfield.cpp \
    modules/config/configmodule.cpp \
    core/core_config/coreconfig.cpp \
    modules/group/groupmodule.cpp \
    core/metadata/groupmetadata.cpp \
    core/metadata/metadata.cpp \
    core/help/modulehelp.cpp \
    core/help/modulehelpusage.cpp \
    core/core_module/coremodule.cpp

HEADERS += \
    bot.h \
    signalhandler.h \
    module.h \
    binputmessage.h \
    database.h \
    botutils.h \
    redis.h \
    modules/board/board.h \
    botinterface.h \
    core/core_data_store/coredatastore.h \
    core/core_model/coremodel.h \
    modules/help/help.h \
    modules/subscribe/subscribe.h \
    model.h \
    modelfield.h \
    modelobject.h \
    modelobjectset.h \
    modelobjectpointer.h \
    botconfig.h \
    configfield.h \
    modules/config/configmodule.h \
    core/core_config/coreconfig.h \
    modules/group/groupmodule.h \
    core/metadata/groupmetadata.h \
    core/metadata/metadata.h \
    core/help/modulehelp.h \
    core/help/modulehelpusage.h \
    core/core_module/coremodule.h

LIBS += -lssl -lcrypto -lz -lqtelegram-ae -lhiredis
INCLUDEPATH += /usr/include/libqtelegram-ae $$OUT_PWD/$$DESTDIR/include/libqtelegram-ae
INCLUDEPATH += /usr/include/openssl/ $$OUT_PWD/$$DESTDIR/include/openssl
