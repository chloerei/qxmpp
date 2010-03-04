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


#ifndef QXMPPSTANZA_H
#define QXMPPSTANZA_H

#include "QXmppElement.h"
#include "QXmppPacket.h"
#include <QString>

// forward declarations of QXmlStream* classes will not work on Mac, we need to
// include the whole header.
// See http://lists.trolltech.com/qt-interest/2008-07/thread00798-0.html
// for an explanation.
#include <QXmlStreamWriter>

class QXmppStanza : public QXmppPacket
{
public:
    class Error
    {
    public:
        enum Type
        {
            Cancel,
            Continue,
            Modify,
            Auth,
            Wait
        };

        enum Condition
        {
            BadRequest,
            Conflict,
            FeatureNotImplemented,
            Forbidden,
            Gone,
            InternalServerError,
            ItemNotFound,
            JidMalformed,
            NotAcceptable,
            NotAllowed,
            NotAuthorized,
            PaymentRequired,
            RecipientUnavailable,
            Redirect,
            RegistrationRequired,
            RemoteServerNotFound,
            RemoteServerTimeout,
            ResourceConstraint,
            ServiceUnavailable,
            SubscriptionRequired,
            UndefinedCondition,
            UnexpectedRequest
        };

        Error();
        Error(Type type, Condition cond, const QString& text="");
        Error(const QString& type, const QString& cond, const QString& text="");

        void setCode(int code);
        void setText(const QString& text);
        void setCondition(Condition cond);
        void setConditionFromStr(const QString& cond);
        void setType(Type type);
        void setTypeFromStr(const QString& type);
        int getCode() const;
        QString getText() const;
        Condition getCondition() const;
        Type getType() const;
        void toXml( QXmlStreamWriter *writer ) const;
        QString getConditionStr() const;
        QString getTypeStr() const;

    private:
        int m_code;
        Type m_type;
        Condition m_condition;
        QString m_text;
    };

    QXmppStanza(const QString& from = "", const QString& to = "");
    ~QXmppStanza();
    
    // deprecated accessors, use the form without "get" instead
    QString Q_DECL_DEPRECATED getTo() const;  
    QString Q_DECL_DEPRECATED getFrom() const;
    QString Q_DECL_DEPRECATED getId() const;    
    QString Q_DECL_DEPRECATED getLang() const;    
    QXmppStanza::Error Q_DECL_DEPRECATED getError() const;

    QString to() const;
    void setTo(const QString&);

    QString from() const;
    void setFrom(const QString&);

    QString id() const;
    void setId(const QString&);

    QString lang() const;
    void setLang(const QString&);

    QXmppStanza::Error error() const;
    void setError(QXmppStanza::Error& error);

    QXmppElementList extensions() const;
    void setExtensions(const QXmppElementList &elements);

    // FIXME : why is this needed?
    bool isErrorStanza();

    // FIXME : make this method protected
    static QXmppStanza::Error parseError(const QDomElement&);

protected:
    void generateAndSetNextId();

private:
    static uint s_uniqeIdNo;
    QString m_to;  
    QString m_from;
    QString m_id;    
    QString m_lang;
    QXmppStanza::Error m_error;
    QXmppElementList m_extensions;
};

#endif // QXMPPSTANZA_H
