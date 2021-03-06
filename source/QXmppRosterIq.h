/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
 *
 * Authors:
 *	Manjeet Dahiya
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


#ifndef QXMPPROSTERIQ_H
#define QXMPPROSTERIQ_H

#include "QXmppIq.h"
#include <QList>
#include <QSet>

class QXmppRosterIq : public QXmppIq
{
public:

    class Item
    {
    public:
        /// An enumeration for type of subscription with the bareJid in the roster.
        enum SubscriptionType
        {
            NotSet = 0,
            None,   ///< the user does not have a subscription to the
                    ///< contact's presence information, and the contact does
                    ///< not have a subscription to the user's presence information
            Both,   ///< both the user and the contact have subscriptions to each
                    ///< other's presence information
            From,   ///< the contact has a subscription to the user's presence information,
                    ///< but the user does not have a subscription to the contact's presence information
            To,     ///< the user has a subscription to the contact's presence information,
                    ///< but the contact does not have a subscription to the user's presence information
            Remove  ///< to delete a roster item
        };

        QString bareJid() const;
        QSet<QString> groups() const;
        QString name() const;
        QString subscriptionStatus() const;
        SubscriptionType subscriptionType() const;

        void setBareJid(const QString&);
        void setGroups(const QSet<QString>&);
        void setName(const QString&);
        void setSubscriptionStatus(const QString&);
        void setSubscriptionType(SubscriptionType);

        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;
        
// deprecated accessors, use the form without "get" instead
        SubscriptionType Q_DECL_DEPRECATED getSubscriptionType() const;
        QString Q_DECL_DEPRECATED getName() const;
        QString Q_DECL_DEPRECATED getSubscriptionStatus() const;
        QString Q_DECL_DEPRECATED getBareJid() const;
        QSet<QString> Q_DECL_DEPRECATED getGroups() const;

    private:
        QString getSubscriptionTypeStr() const;
        void setSubscriptionTypeFromStr(const QString&);

        QString m_bareJid;
        SubscriptionType m_type;
        QString m_name;
         // can be subscribe/unsubscribe (attribute "ask")
        QString m_subscriptionStatus;
        QSet<QString> m_groups;
    };

    void addItem(const Item&);
    QList<Item> items() const;

    static bool isRosterIq(const QDomElement &element);
    void parse(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

// deprecated accessors, use the form without "get" instead
    QList<Item> Q_DECL_DEPRECATED getItems() const;

private:
    QList<Item> m_items;
};

#endif // QXMPPROSTERIQ_H
