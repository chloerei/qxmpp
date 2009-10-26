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


#include "QXmppRosterIq.h"
#include "QXmppConstants.h"
#include "QXmppUtils.h"
#include <QXmlStreamWriter>

QXmppRosterIq::QXmppRosterIq(QXmppIq::Type type)
    : QXmppIq(type)
{

}

QXmppRosterIq::QXmppRosterIq(const QString& type)
    : QXmppIq(type)
{
}

QXmppRosterIq::~QXmppRosterIq()
{

}

void QXmppRosterIq::addItem(const Item& item)
{
    m_items.append(item);
}

QList<QXmppRosterIq::Item> QXmppRosterIq::getItems() const
{
    return m_items;
}

void QXmppRosterIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute( "xmlns", ns_roster);

    for(int i = 0; i < m_items.count(); ++i)
        m_items.at(i).toXml(writer);
    writer->writeEndElement();
}

QXmppRosterIq::Item::SubscriptionType
        QXmppRosterIq::Item::getSubscriptionType() const
{
    return m_type;
}

QString QXmppRosterIq::Item::getName() const
{
    return m_name;
}

QString QXmppRosterIq::Item::getSubscriptionStatus() const
{
    return m_subscriptionStatus;
}

QString QXmppRosterIq::Item::getBareJid() const
{
    return m_bareJid;
}

QSet<QString> QXmppRosterIq::Item::getGroups() const
{
    return m_groups;
}

void QXmppRosterIq::Item::setName(const QString& str)
{
    m_name = str;
}

void QXmppRosterIq::Item::setSubscriptionStatus(const QString& str)
{
    m_subscriptionStatus = str;
}

void QXmppRosterIq::Item::addGroup(const QString& str)
{
    m_groups << str;
}

void QXmppRosterIq::Item::setBareJid(const QString& str)
{
    m_bareJid = str;
}

QString QXmppRosterIq::Item::getSubscriptionTypeStr() const
{
    switch(m_type)
    {
    case NotSet:
        return "";
    case None:
        return "none";
    case Both:
        return "both";
    case From:
        return "from";
    case To:
        return "to";
    case Remove:
        return "remove";
    default:
        {
            qWarning("QXmppRosterIq::Item::getTypeStr(): invalid type");
            return "";
        }
    }
}

void QXmppRosterIq::Item::setSubscriptionType(SubscriptionType type)
{
    m_type = type;
}

void QXmppRosterIq::Item::setSubscriptionTypeFromStr(const QString& type)
{
    if(type == "none")
        setSubscriptionType(None);
    else if(type == "both")
        setSubscriptionType(Both);
    else if(type == "from")
        setSubscriptionType(From);
    else if(type == "to")
        setSubscriptionType(To);
    else if(type == "remove")
        setSubscriptionType(Remove);
    else
        qWarning("QXmppRosterIq::Item::setTypeFromStr(): invalid type");
}

void QXmppRosterIq::Item::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("item");
    helperToXmlAddAttribute(writer,"jid", m_bareJid);
    helperToXmlAddAttribute(writer,"name", m_name);
    helperToXmlAddAttribute(writer,"subscription", getSubscriptionTypeStr());
    helperToXmlAddAttribute(writer, "ask", getSubscriptionStatus());

    QSet<QString>::const_iterator i = m_groups.constBegin();
    while(i != m_groups.constEnd())
    {
        helperToXmlAddTextElement(writer,"group", *i);
        ++i;
    }
    writer->writeEndElement();
}
