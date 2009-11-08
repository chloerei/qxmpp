#include "remoteinterface.h"

RemoteInterface::RemoteInterface(QObject *parent) : QXmppInvokable(parent)
{
}

bool RemoteInterface::isAuthorized( const QString &jid ) const
{
    return true;
}

QString RemoteInterface::echoString(const QString& message)
{
    return "Echo: " + message;
}
