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

QMAKE_CXXFLAGS += -std=c++11


SOURCES += main.cpp \
    bot.cpp \
    signalhandler.cpp \
    module.cpp \
    binputmessage.cpp \
    database.cpp \
    botutils.cpp \
    redis.cpp \
    modules/board/board.cpp \
    botinterface.cpp

HEADERS += \
    bot.h \
    signalhandler.h \
    module.h \
    binputmessage.h \
    database.h \
    botutils.h \
    redis.h \
    modules/board/board.h \
    modules/board/boardmodel.h \
    model.h \
    modules/board/boarditemmodel.h \
    botinterface.h

LIBS += -lssl -lcrypto -lz -lqtelegram-ae -lhiredis
INCLUDEPATH += /usr/include/libqtelegram-ae $$OUT_PWD/$$DESTDIR/include/libqtelegram-ae
INCLUDEPATH += /usr/include/openssl/ $$OUT_PWD/$$DESTDIR/include/openssl
