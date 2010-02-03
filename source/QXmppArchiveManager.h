/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifndef QXMPPARCHIVEMANAGER_H
#define QXMPPARCHIVEMANAGER_H

#include <QDateTime>
#include <QObject>

class QXmppArchiveChat;
class QXmppArchiveChatIq;
class QXmppArchiveListIq;
class QXmppArchivePrefIq;
class QXmppClient;

class QXmppArchiveManager : public QObject
{
    Q_OBJECT

public:
    QXmppArchiveManager(QXmppClient* client);
    void listCollections(const QString &jid, const QDateTime &start = QDateTime(), const QDateTime &end = QDateTime(), int max = 0);
    void retrieveCollection(const QString &jid, const QDateTime &start, int max = 0);
    void getPreferences();

signals:
    void archiveListReceived(const QList<QXmppArchiveChat>&);
    void archiveChatReceived(const QXmppArchiveChat&);

private slots:
    void archiveChatIqReceived(const QXmppArchiveChatIq&);
    void archiveListIqReceived(const QXmppArchiveListIq&);
    void archivePrefIqReceived(const QXmppArchivePrefIq&);

private:
    // reference to to client object (no ownership)
    QXmppClient* m_client;
};

#endif
