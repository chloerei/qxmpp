/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
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


#include "QXmppVCard.h"
#include "QXmppUtils.h"
#include "QXmppConstants.h"

#include <QXmlStreamWriter>
#include <QImage>
#include <QBuffer>

QXmppVCard::QXmppVCard(const QString& jid) : QXmppIq(QXmppIq::Get)
{
    // for self jid should be empty
    setTo(jid);
}

QString QXmppVCard::firstName() const
{
    return m_firstName;
}

void QXmppVCard::setFirstName(const QString &firstName)
{
    m_firstName = firstName;
}

QString QXmppVCard::fullName() const
{
    return m_fullName;
}

void QXmppVCard::setFullName(const QString& str)
{
    m_fullName = str;
}

QString QXmppVCard::lastName() const
{
    return m_lastName;
}

void QXmppVCard::setLastName(const QString &lastName)
{
    m_lastName = lastName;
}

QString QXmppVCard::middleName() const
{
    return m_middleName;
}

void QXmppVCard::setMiddleName(const QString &middleName)
{
    m_middleName = middleName;
}

QString QXmppVCard::nickName() const
{
    return m_nickName;
}

void QXmppVCard::setNickName(const QString& str)
{
    m_nickName = str;
}

QString QXmppVCard::url() const
{
    return m_url;
}

void QXmppVCard::setUrl(const QString& url)
{
    m_url = url;
}

const QByteArray& QXmppVCard::photo() const
{
    return m_photo;
}

void QXmppVCard::setPhoto(const QByteArray& photo)
{
    m_photo = photo;
}

void QXmppVCard::setPhoto(const QImage& image)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    m_photo = ba;
}

void QXmppVCard::parse(const QDomElement& nodeRecv)
{
    QXmppStanza::parse(nodeRecv);

    setTypeFromStr(nodeRecv.attribute("type"));

    // vCard
    QDomElement cardElement = nodeRecv.firstChildElement("vCard");
    m_fullName = cardElement.firstChildElement("FN").text();
    m_nickName = cardElement.firstChildElement("NICKNAME").text();
    QDomElement nameElement = cardElement.firstChildElement("N");
    m_firstName = nameElement.firstChildElement("GIVEN").text();
    m_lastName = nameElement.firstChildElement("FAMILY").text();
    m_middleName = nameElement.firstChildElement("MIDDLE").text();
    m_url = cardElement.firstChildElement("URL").text();
    QByteArray base64data = cardElement.
                            firstChildElement("PHOTO").
                            firstChildElement("BINVAL").text().toAscii();
    setPhoto(QByteArray::fromBase64(base64data));
}

void QXmppVCard::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("vCard");
    helperToXmlAddAttribute(writer,"xmlns", ns_vcard);
    if (!m_fullName.isEmpty())
        helperToXmlAddTextElement(writer, "FN", m_fullName);
    if(!m_nickName.isEmpty())
        helperToXmlAddTextElement(writer, "NICKNAME", m_nickName);
    if (!m_firstName.isEmpty() ||
        !m_lastName.isEmpty() ||
        !m_middleName.isEmpty())
    {
        writer->writeStartElement("N");
        if (!m_firstName.isEmpty())
            helperToXmlAddTextElement(writer, "GIVEN", m_firstName);
        if (!m_lastName.isEmpty())
            helperToXmlAddTextElement(writer, "FAMILY", m_lastName);
        if (!m_middleName.isEmpty())
            helperToXmlAddTextElement(writer, "MIDDLE", m_middleName);
        writer->writeEndElement();
    }
    if (!m_url.isEmpty())
        helperToXmlAddTextElement(writer, "URL", m_url);

    if(!photo().isEmpty())
    {
        writer->writeStartElement("PHOTO");
        helperToXmlAddTextElement(writer, "TYPE", getImageType(photo()));
        helperToXmlAddTextElement(writer, "BINVAL", photo().toBase64());
        writer->writeEndElement();
    }

    writer->writeEndElement();
}

QImage QXmppVCard::photoAsImage() const
{
    return getImageFromByteArray(photo());
}

QString QXmppVCard::getFullName() const
{
    return m_fullName;
}

QString QXmppVCard::getNickName() const
{
    return m_nickName;
}

const QByteArray& QXmppVCard::getPhoto() const
{
    return m_photo;
}

QImage QXmppVCard::getPhotoAsImage() const
{
    return getImageFromByteArray(photo());
}

