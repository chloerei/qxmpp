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

#include "QXmppElement.h"
#include "QXmppUtils.h"

#include <QDomElement>

class QXmppElementPrivate
{
public:
    QXmppElementPrivate();
    QXmppElementPrivate(const QDomElement &element);
    ~QXmppElementPrivate();

    QAtomicInt counter;

    QMap<QString, QString> attributes;
    QList<QXmppElementPrivate*> children;
    QString name;
    QString value;
};

QXmppElementPrivate::QXmppElementPrivate()
    : counter(1)
{
}

QXmppElementPrivate::QXmppElementPrivate(const QDomElement &element)
    : counter(1)
{
    if (element.isNull())
        return;

    name = element.tagName();
    QString xmlns = element.namespaceURI();
    QString parentns = element.parentNode().namespaceURI();
    if (!xmlns.isEmpty() && xmlns != parentns)
        attributes.insert("xmlns", xmlns);
    QDomNamedNodeMap attrs = element.attributes();
    for (int i = 0; i < attrs.size(); i++)
    {
        QDomAttr attr = attrs.item(i).toAttr();
        attributes.insert(attr.name(), attr.value());
    }

    QDomNode childNode = element.firstChild();
    while (!childNode.isNull())
    {
        if (childNode.isElement())
            children.append(new QXmppElementPrivate(childNode.toElement()));
        else if (childNode.isText())
            value += childNode.toText().data();
        childNode = childNode.nextSibling();
    }
}

QXmppElementPrivate::~QXmppElementPrivate()
{
    foreach (QXmppElementPrivate *child, children)
        if (!child->counter.deref())
            delete child;
}

QXmppElement::QXmppElement()
{
    d = new QXmppElementPrivate();
}

QXmppElement::QXmppElement(const QXmppElement &other)
{
    other.d->counter.ref();
    d = other.d;
}

QXmppElement::QXmppElement(QXmppElementPrivate *other)
{
    other->counter.ref();
    d = other;
}

QXmppElement::QXmppElement(const QDomElement &element)
{
    d = new QXmppElementPrivate(element);
}

QXmppElement::~QXmppElement()
{
    if (!d->counter.deref())
        delete d;
}

QXmppElement &QXmppElement::operator=(const QXmppElement &other)
{
    other.d->counter.ref();
    if (!d->counter.deref())
        delete d;
    d = other.d;
    return *this;
}

QStringList QXmppElement::attributeNames() const
{
    return d->attributes.keys();
}

QString QXmppElement::attribute(const QString &name) const
{
    return d->attributes.value(name);
}

void QXmppElement::setAttribute(const QString &name, const QString &value)
{
    d->attributes.insert(name, value);
}

void QXmppElement::appendChild(const QXmppElement &child)
{
    if (!d->children.contains(child.d))
    {
        child.d->counter.ref();
        d->children.append(child.d);
    }
}

QXmppElementList QXmppElement::children() const
{
    QXmppElementList list;
    foreach (QXmppElementPrivate *child_d, d->children)
        list.append(QXmppElement(child_d));
    return list;
}

QXmppElement QXmppElement::firstChildElement(const QString &name) const
{
    foreach (const QXmppElement &child, d->children)
        if (name.isEmpty() || child.tagName() == name)
            return child;
    return QXmppElement();
}

bool QXmppElement::isNull() const
{
    return d->name.isEmpty();
}

void QXmppElement::removeChild(const QXmppElement &child)
{
    if (d->children.contains(child.d))
    {
        d->children.removeAll(child.d);
        child.d->counter.deref();
    }
}

QString QXmppElement::tagName() const
{
    return d->name;
}

void QXmppElement::setTagName(const QString &tagName)
{
    d->name = tagName;
}

QString QXmppElement::value() const
{
    return d->value;
}

void QXmppElement::setValue(const QString &value)
{
    d->value = value;
}

void QXmppElement::toXml(QXmlStreamWriter *writer) const
{
    if (isNull())
        return;

    writer->writeStartElement(d->name);
    foreach (const QString &attr, d->attributes.keys())
        helperToXmlAddAttribute(writer, attr, d->attributes.value(attr));
    writer->writeCharacters(d->value);
    foreach (const QXmppElement &child, d->children)
        child.toXml(writer);
    writer->writeEndElement();
}

QXmppElementList::QXmppElementList()
{
}

QXmppElementList::QXmppElementList(const QXmppElement &element)
{
    append(element);
}


QXmppElementList::QXmppElementList(const QList<QXmppElement> &other)
    : QList<QXmppElement>(other)
{
}

