TEMPLATE = lib
QT += network \
    xml
CONFIG += staticlib \
    debug_and_release
CONFIG(debug, debug|release):TARGET = QXmppClient_d
else:TARGET = QXmppClient

# Header files
HEADERS += utils.h \
    QXmppBind.h \
    QXmppClient.h \
    QXmppConfiguration.h \
    QXmppConstants.h \
    QXmppIq.h \
    QXmppMessage.h \
    QXmppPacket.h \
    QXmppPresence.h \
    QXmppRoster.h \
    QXmppRosterIq.h \
    QXmppSession.h \
    QXmppStanza.h \
    QXmppStream.h \
    QXmppLogger.h \
    QXmppReconnectionManager.h

# Source files
SOURCES += utils.cpp \
    QXmppBind.cpp \
    QXmppClient.cpp \
    QXmppConfiguration.cpp \
    QXmppConstants.cpp \
    QXmppIq.cpp \
    QXmppMessage.cpp \
    QXmppPacket.cpp \
    QXmppPresence.cpp \
    QXmppRoster.cpp \
    QXmppRosterIq.cpp \
    QXmppSession.cpp \
    QXmppStanza.cpp \
    QXmppStream.cpp \
    QXmppLogger.cpp \
    QXmppReconnectionManager.cpp
