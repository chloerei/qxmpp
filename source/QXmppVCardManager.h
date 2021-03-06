/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
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


#ifndef QXMPPVCARDMANAGER_H
#define QXMPPVCARDMANAGER_H

#include <QObject>
#include "QXmppClient.h"
#include "QXmppVCard.h"

class QXmppVCardManager : public QObject
{
    Q_OBJECT

public:
    QXmppVCardManager(QXmppClient* client);
    void requestVCard(const QString& bareJid = "");

    const QXmppVCard& clientVCard() const;
    void setClientVCard(const QXmppVCard&);
    void requestClientVCard();
    bool isClientVCardReceived();

signals:
    void vCardReceived(const QXmppVCard&);
    void clientVCardReceived();

private slots:
    void vCardIqReceived(const QXmppVCard&);

private:
    // reference to to client object (no ownership)
    QXmppClient* m_client;

    QXmppVCard m_clientVCard;  ///< Stores the vCard of the connected client
    bool m_isClientVCardReceived;
};

#endif // QXMPPVCARDMANAGER_H
