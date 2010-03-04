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

#ifndef QXMPPVERSIONIQ_H
#define QXMPPVERSIONIQ_H

#include "QXmppIq.h"

/// XEP-0092: Software Version
/// http://xmpp.org/extensions/xep-0092.html

class QXmppVersionIq : public QXmppIq
{
public:
    QString name() const;
    void setName(const QString &name);

    QString os() const;
    void setOs(const QString &os);

    QString version() const;
    void setVersion(const QString &version);

    static bool isVersionIq(const QDomElement &element);
    void parse(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QString m_name;
    QString m_os;
    QString m_version;
};

#endif
