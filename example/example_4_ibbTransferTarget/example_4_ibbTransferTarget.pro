TEMPLATE = app

TARGET = example_4_ibbTransferTarget

SOURCES +=  main.cpp \
            ibbTransferTarget.cpp

HEADERS +=  ibbTransferTarget.h

INCLUDEPATH += ../../source

QT += network xml
CONFIG += console debug_and_release

CONFIG(debug, debug|release) {
    LIBS += -L../../source/debug -lQXmppClient_d
 } else {
    LIBS += -L../../source/release -lQXmppClient
 }

OTHER_FILES += README
