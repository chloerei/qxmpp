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


#include "QXmppStream.h"
#include "QXmppPacket.h"
#include "QXmppUtils.h"
#include "QXmppClient.h"
#include "QXmppRoster.h"
#include "QXmppPresence.h"
#include "QXmppIq.h"
#include "QXmppBind.h"
#include "QXmppSession.h"
#include "QXmppRosterIq.h"
#include "QXmppMessage.h"
#include "QXmppConstants.h"
#include "QXmppVCard.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppInformationRequestResult.h"
#include "QXmppLogger.h"

#include <QDomDocument>
#include <QStringList>
#include <QRegExp>
#include <QHostAddress>
#include <QXmlStreamWriter>

static const QByteArray streamRootElementStart = "<?xml version=\"1.0\"?><stream:stream xmlns:stream=\"http://etherx.jabber.org/streams\" version=\"1.0\" xmlns=\"jabber:client\" xml:lang=\"en\" xmlns:xml=\"http://www.w3.org/XML/1998/namespace\">\n";
static const QByteArray streamRootElementEnd = "</stream:stream>";

QXmppStream::QXmppStream(QXmppClient* client)
    : QObject(client), m_roster(this), m_client(client),
    m_sessionAvaliable(false), m_vCardManager(m_client)
{
    bool check = QObject::connect(&m_socket, SIGNAL(hostFound()),
                                  this, SLOT(socketHostFound()));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket, SIGNAL(connected()),
                             this, SLOT(socketConnected()));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket, SIGNAL(disconnected()),
                             this, SLOT(socketDisconnected()));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket, SIGNAL(readyRead()),
                             this, SLOT(socketReadReady()));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket, SIGNAL(encrypted()),
                             this, SLOT(socketEncrypted()));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket,
                             SIGNAL(sslErrors(const QList<QSslError>&)), this,
                             SLOT(socketSslErrors(const QList<QSslError>&)));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket,
                             SIGNAL(error(QAbstractSocket::SocketError)), this,
                             SLOT(socketError(QAbstractSocket::SocketError)));
    Q_ASSERT(check);

    check = QObject::connect(this,
                            SIGNAL(presenceReceived(const QXmppPresence&)),
                            &m_roster,
                            SLOT(presenceReceived(const QXmppPresence&)));
    Q_ASSERT(check);
    
    check = QObject::connect(this, SIGNAL(rosterIqReceived(const QXmppRosterIq&)), 
        &m_roster, SLOT(rosterIqReceived(const QXmppRosterIq&)));
    Q_ASSERT(check);

    check = QObject::connect(this, SIGNAL(vCardIqReceived(const QXmppVCard&)),
        &m_vCardManager, SLOT(vCardIqReceived(const QXmppVCard&)));
    Q_ASSERT(check);
}

QXmppStream::~QXmppStream()
{

}

QXmppConfiguration& QXmppStream::getConfiguration()
{
    return m_client->getConfiguration();
}

void QXmppStream::connect()
{
    // work with time out
    log(QString("Connecting to: %1:%2").arg(getConfiguration().
            getHost()).arg(getConfiguration().getPort()));
    m_socket.connectToHost(getConfiguration().
                           getHost(), getConfiguration().getPort());
}

void QXmppStream::socketSslErrors(const QList<QSslError> & error)
{
    log(QString("SSL errors"));
    m_socket.ignoreSslErrors();
    for(int i = 0; i< error.count(); ++i)
        log(error.at(i).errorString());
}

void QXmppStream::socketHostFound()
{
    log(QString("Host found"));
    emit hostFound();
}

void QXmppStream::socketConnected()
{
    flushDataBuffer();
    log(QString("Connected"));
    emit connected();
    sendStartStream();
}

void QXmppStream::socketDisconnected()
{
    flushDataBuffer();
    log(QString("Disconnected"));
    emit disconnected();
}

void QXmppStream::socketEncrypted()
{
    log(QString("Encrypted"));
    sendStartStream();
}

void QXmppStream::socketError(QAbstractSocket::SocketError ee)
{
    m_socketError = ee;
    emit error(QXmppClient::SocketError);
    log(QString("Socket error: " + m_socket.errorString()));
}

void QXmppStream::socketReadReady()
{
    QByteArray data = m_socket.readAll();
    log("SERVER [COULD BE PARTIAL DATA]:" + data.left(20));
    parser(data);
}

void QXmppStream::sendNonSASLAuthQuery( const QString &to )
{
    QXmppNonSASLAuthTypesRequestIq authQuery;
    authQuery.setTo(to);
    authQuery.setUsername(getConfiguration().getUser());

    sendPacket(authQuery);
}

void QXmppStream::parser(const QByteArray& data)
{
    QDomDocument doc;
    QByteArray completeXml;

    m_dataBuffer = m_dataBuffer + data;

    if(hasStartStreamElement(m_dataBuffer))
    {
        completeXml = m_dataBuffer + streamRootElementEnd;
    }
    else if(hasEndStreamElement(data))
    {
        completeXml = streamRootElementStart + m_dataBuffer;
    }
    else
    {
        completeXml = streamRootElementStart + m_dataBuffer + streamRootElementEnd;
    }
    
    if(doc.setContent(completeXml, true))
    {
        log("SERVER:" + m_dataBuffer);
        flushDataBuffer();

        QDomElement nodeRecv = doc.documentElement().firstChildElement();

        if(nodeRecv.isNull())
        {
            QDomElement streamElement = doc.documentElement();
            if(m_streamId.isEmpty())
                m_streamId = streamElement.attribute("id");
            if(m_XMPPVersion.isEmpty())
            {
                m_XMPPVersion = streamElement.attribute("version");
                if(m_XMPPVersion.isEmpty())
                {
                    // no version specified, signals XMPP Version < 1.0.
                    // switch to old auth mechanism
                    sendNonSASLAuthQuery(doc.documentElement().attribute("from"));
                }
            }
        }
        else
        {
            //TODO: Make a login error here.
        }

        while(!nodeRecv.isNull())
        {
            QString ns = nodeRecv.namespaceURI();
            if(ns == ns_stream && nodeRecv.tagName() == "features")
            {
                bool nonSaslAvailable = nodeRecv.firstChildElement("auth").
                                         namespaceURI() == ns_authFeature;
                bool saslAvailable = nodeRecv.firstChildElement("mechanisms").
                                     namespaceURI() == ns_sasl;
                bool useSasl = getConfiguration().getUseSASLAuthentication();

                if(nodeRecv.firstChildElement("starttls").
                                     namespaceURI() == ns_tls &&
                                     nodeRecv.firstChildElement("starttls").
                                     firstChildElement().tagName() == "required")
                {
                    sendStartTls();
                    return;
                }

                if((saslAvailable && nonSaslAvailable && !useSasl) ||
                   (!saslAvailable && nonSaslAvailable))
                {
                    sendNonSASLAuthQuery(doc.documentElement().attribute("from"));
                }
                else if(saslAvailable)
                {
                    // SASL Authentication
                    QDomElement element = nodeRecv.firstChildElement("mechanisms");
                    log(QString("Mechanisms:"));
                    QDomElement subElement = element.firstChildElement();
                    QStringList mechanisms;
                    while(!subElement.isNull())
                    {
                        if(subElement.tagName() == "mechanism")
                        {
                            log(subElement.text());
                            mechanisms << subElement.text();
                        }
                        subElement = subElement.nextSiblingElement();
                    }
                    sendAuthPlain();
                }

                if(nodeRecv.firstChildElement("bind").
                                     namespaceURI() == ns_bind)
                {
                    sendBindIQ();
                }

                if(nodeRecv.firstChildElement("session").
                                     namespaceURI() == ns_session)
                {
                    m_sessionAvaliable = true;
                }
            }
            else if(ns == ns_tls)
            {
                if(nodeRecv.tagName() == "proceed")
                {
                    log(QString("Starting encryption"));
                    m_socket.startClientEncryption();
                    return;
                }
            }
            else if(ns == ns_sasl)
            {
                if(nodeRecv.tagName() == "success")
                {
                    log(QString("Authenticated"));
                    sendStartStream();
                }
            }
            else if(ns == ns_client)
            {

                if(nodeRecv.tagName() == "iq")
                {
                    QDomElement element = nodeRecv.firstChildElement();
                    QString id = nodeRecv.attribute("id");
                    QString to = nodeRecv.attribute("to");
                    QString from = nodeRecv.attribute("from");
                    QString type = nodeRecv.attribute("type");
                    if(type.isEmpty())
                        qWarning("QXmppStream: iq type can't be empty");
                    QXmppIq iqPacket;    // to emit

                    
                    QDomElement elemen = nodeRecv.firstChildElement("error");
                    QXmppStanza::Error error = parseStanzaError(elemen);


                    if(id == m_sessionId)
                    {
                        // get back add configuration whether to send
                        // roster and intial presence in beginning
                        // process SessionIq

                        // xmpp connection made
                        emit xmppConnected();

                        sendRosterRequest();
                        sendInitialPresence();

                        QXmppBind session(type);
                        session.setId(id);
                        session.setTo(to);
                        session.setFrom(from);
                        iqPacket = session;
                    }
                    else if(id == m_bindId)
                    {
                        QXmppBind bind(type);
                        QString jid = nodeRecv.firstChildElement("bind").
                                      firstChildElement("jid").text();
                        bind.setResource(jidToResource(jid));
                        bind.setJid(jidToBareJid(jid));
                        bind.setId(id);
                        bind.setTo(to);
                        bind.setFrom(from);
                        processBindIq(bind);
                        iqPacket = bind;
                    }
                    else if(nodeRecv.firstChildElement("query").
                            namespaceURI() == ns_roster)
                    {
                        QDomElement itemElement = nodeRecv.
                                                  firstChildElement("query").
                                                  firstChildElement("item");
                        QXmppRosterIq rosterIq(nodeRecv.attribute("type"));
                        rosterIq.setId(id);
                        rosterIq.setTo(to);
                        rosterIq.setFrom(from);
                        while(!itemElement.isNull())
                        {
                            QXmppRosterIq::Item item;
                            item.setName(itemElement.attribute("name"));
                            item.setBareJid(itemElement.attribute("jid"));
                            item.setSubscriptionTypeFromStr(
                                    itemElement.attribute("subscription"));
                            item.setSubscriptionStatus(
                                    itemElement.attribute("ask"));
                            item.addGroup(
                                    itemElement.firstChildElement("group").firstChildElement().text());
                            rosterIq.addItem(item);
                            itemElement = itemElement.nextSiblingElement();
                        }
                        processRosterIq(rosterIq);
                        iqPacket = rosterIq;
                    }
                    // extensions
                    // vCard - XEP-0054
                    // http://xmpp.org/extensions/xep-0054.html
                    else if(nodeRecv.firstChildElement("vCard").
                            namespaceURI() == ns_vcard)
                    {
                        QXmppVCard vcardIq;
                        vcardIq.parse(nodeRecv);
                        emit vCardIqReceived(vcardIq);
                        iqPacket = vcardIq;
                    }
                    // XEP-0030 info query
                    else if(nodeRecv.firstChildElement("query").
                            namespaceURI() == ns_disco_info &&
                            type == "get")
                    {
                        QXmppInformationRequestResult qxmppFeatures;
                        qxmppFeatures.setId(id);
                        qxmppFeatures.setTo(from);
                        qxmppFeatures.setFrom(to);
                        sendPacket(qxmppFeatures);
                    }
                    else if(id == m_nonSASLAuthId && type == "result")
                    {
                        // successful Non-SASL Authentication
                        log(QString("Authenticated (Non-SASL)"));

                        emit xmppConnected();

                        sendRosterRequest();
                        sendInitialPresence();
                    }
                    else if(nodeRecv.firstChildElement("query").
                            namespaceURI() == ns_auth)
                    {
                        if(type == "result")
                        {
                            bool plainText = false;
                            if ( nodeRecv.firstChildElement("query").
                                 firstChildElement("digest").isNull() )
                                plainText = true;
                            sendNonSASLAuth(plainText);
                        }
                    }
                    else // didn't understant the iq...reply with error
                    {
                        if(type != "result") // but not incase of result iqs
                        {
                            QXmppIq iq(QXmppIq::Error);
                            iq.setId(id);
                            iq.setTo(from);
                            iq.setFrom(to);
                            QXmppStanza::Error error(QXmppStanza::Error::Cancel,
                                QXmppStanza::Error::FeatureNotImplemented);
                            iq.setError(error);
                            sendPacket(iq);
                        }
                    }

                    iqPacket.setError(error);
                    processIq(iqPacket);
                }
                else if(nodeRecv.tagName() == "presence")
                {
                    QXmppPresence presence;
                    presence.setTypeFromStr(nodeRecv.attribute("type"));
                    presence.setFrom(nodeRecv.attribute("from"));
                    presence.setTo(nodeRecv.attribute("to"));
                    
                    QString statusText = nodeRecv.
                                         firstChildElement("status").text();
                    QString show = nodeRecv.
                                   firstChildElement("show").text();
                    int priority = nodeRecv.
                                   firstChildElement("priority").text().toInt();
                    QXmppPresence::Status status;
                    status.setTypeFromStr(show);
                    status.setStatusText(statusText);
                    status.setPriority(priority);
                    presence.setStatus(status);

                    QDomElement errorElement = nodeRecv.
                                               firstChildElement("error");
                    if(!errorElement.isNull())
                    {
                        QXmppStanza::Error error =
                                parseStanzaError(errorElement);
                        presence.setError(error);
                    }

                    processPresence(presence);
                }
                else if(nodeRecv.tagName() == "message")
                {
                    QString from = nodeRecv.attribute("from");
                    QString to = nodeRecv.attribute("to");
                    QString type = nodeRecv.attribute("type");
                    QString body = unescapeString(
                            nodeRecv.firstChildElement("body").text());
                    QString sub = unescapeString(
                            nodeRecv.firstChildElement("subject").text());
                    QString thread = nodeRecv.firstChildElement("thread").text();
                    QXmppMessage message(from, to, body, thread);
                    message.setSubject(sub);
                    message.setTypeFromStr(type);

                    QDomElement errorElement = nodeRecv.
                                               firstChildElement("error");
                    if(!errorElement.isNull())
                    {
                        QXmppStanza::Error error = parseStanzaError(errorElement);
                        message.setError(error);
                    }
                    processMessage(message);
                }
            }
            nodeRecv = nodeRecv.nextSiblingElement();
        }
    }
    else
    {
        //wait for complete packet
    }
}


void QXmppStream::sendStartStream()
{
    QByteArray data = "<?xml version='1.0'?><stream:stream to='";
    data.append(getConfiguration().getDomain());
    data.append("' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>");
    sendToServer(data);
}

void QXmppStream::sendToServer(const QByteArray& packet)
{
    m_socket.write( packet );
}

bool QXmppStream::hasStartStreamElement(const QByteArray& data)
{
    QString str(data);
    QRegExp regex("(<\\?xml.*\\?>)?\\s*<stream:stream.*>");
    regex.setMinimal(true);
    if(str.contains(regex))
        return true;
    else
        return false;
}

bool QXmppStream::hasEndStreamElement(const QByteArray& data)
{
    QString str(data);
    QRegExp regex("</stream:stream>");
    regex.setMinimal(true);
    if(str.contains(regex))
        return true;
    else
        return false;
}

void QXmppStream::sendStartTls()
{
    sendToServer("<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
}

void QXmppStream::sendNonSASLAuth(bool plainText)
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setUsername(getConfiguration().getUser());
    authQuery.setPassword(getConfiguration().getPasswd());
    authQuery.setResource(getConfiguration().getResource());
    authQuery.setStreamId(m_streamId);
    authQuery.setUsePlainText(plainText);
    m_nonSASLAuthId = authQuery.getId();
    sendPacket(authQuery);
}

void QXmppStream::sendAuthPlain()
{
    QByteArray data = "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'>";
    QString userPass('\0' + getConfiguration().getUser() +
                     '\0' + getConfiguration().getPasswd());
    data += userPass.toUtf8().toBase64();
    data += "</auth>";
    sendToServer(data);
}

void QXmppStream::sendBindIQ()
{
    QXmppBind bind(QXmppIq::Set);
    bind.setResource(getConfiguration().getResource());
    m_bindId = bind.getId();
    sendPacket(bind);
}

void QXmppStream::sendSessionIQ()
{
    QXmppSession session(QXmppIq::Set);
    session.setTo(getConfiguration().getDomain());
    m_sessionId = session.getId();
    sendPacket(session);
}

void QXmppStream::sendInitialPresence()
{
    if(m_client)
        sendPacket(m_client->getClientPresence());
}

void QXmppStream::acceptSubscriptionRequest(const QString& from, bool accept)
{
    QXmppPresence presence;
    presence.setTo(from);
    if(accept)
        presence.setType(QXmppPresence::Subscribed);
    else
        presence.setType(QXmppPresence::Unsubscribed);

    sendPacket(presence);
}

void QXmppStream::sendSubscriptionRequest(const QString& to)
{
    if(to.isEmpty())
        return;

    QXmppPresence presence;
    presence.setTo(to);
    presence.setType(QXmppPresence::Subscribe);
    sendPacket(presence);
}

void QXmppStream::sendRosterRequest()
{
    QXmppRosterIq roster(QXmppIq::Get);
    roster.setFrom(getConfiguration().getJid());
    m_rosterReqId = roster.getId();
    sendPacket(roster);
}

void QXmppStream::disconnect()
{
    sendEndStream();
    m_socket.disconnectFromHost();
}

QXmppRoster& QXmppStream::getRoster()
{
    return m_roster;
}

void QXmppStream::sendPacket(const QXmppPacket& packet)
{
    if(QXmppLogger::getLogger()->getLoggingType() != QXmppLogger::NONE)
    {
        QByteArray logPacket;
        QXmlStreamWriter xmlStreamLog(&logPacket);
        packet.toXml(&xmlStreamLog);
        log("CLIENT: "+ logPacket);
    }

    QXmlStreamWriter xmlStream(&m_socket);
    packet.toXml(&xmlStream);
}

void QXmppStream::processPresence(const QXmppPresence& presence)
{
    switch(presence.getType())
    {
    case QXmppPresence::Error:
        break;
    case QXmppPresence::Available:
        break;
    case QXmppPresence::Unavailable:
        break;
    case QXmppPresence::Subscribe:
        if(!presence.getFrom().isEmpty())
        {
            if(getConfiguration().getAutoAcceptSubscriptions())
                acceptSubscriptionRequest(presence.getFrom());
            emit subscriptionRequestReceived(presence.getFrom());
        }
        break;
    case QXmppPresence::Unsubscribe:
        break;
    case QXmppPresence::Unsubscribed:
        break;
    case QXmppPresence::Probe:
        break;
    default:
        break;
    }
    emit presenceReceived(presence);
}

void QXmppStream::processMessage(const QXmppMessage& message)
{
    emit messageReceived(message);
}

void QXmppStream::processIq(const QXmppIq& iq)
{
    emit iqReceived(iq);
}

void QXmppStream::sendEndStream()
{
    sendToServer(streamRootElementEnd);
}

void QXmppStream::processBindIq(const QXmppBind& bind)
{
    switch(bind.getType())
    {
    case QXmppIq::Result:
        if(!bind.getResource().isEmpty())
            getConfiguration().setResource(bind.getResource());
        if(m_sessionAvaliable)
            sendSessionIQ();
        break;
    default:
        break;
    }
}

void QXmppStream::processRosterIq(const QXmppRosterIq& rosterIq)
{
    emit rosterIqReceived(rosterIq);
    switch(rosterIq.getType())
    {
    case QXmppIq::Set:
        // when contact subscribes user...user sends 'subscribed' presence 
        // then after recieving following iq user requests contact for subscription
        
        // check thet "from" is newly added in the roster...and remove this ask thing...and do this for all items
        if(rosterIq.getItems().at(0).getSubscriptionType() ==
           QXmppRosterIq::Item::From && rosterIq.getItems().at(0).
           getSubscriptionStatus().isEmpty())
            sendSubscriptionRequest(rosterIq.getItems().at(0).getBareJid());
        break;
    default:
        break;
    }
}

QXmppStanza::Error QXmppStream::parseStanzaError(QDomElement & errorElement)
{
    QXmppStanza::Error error;
 
    if(errorElement.isNull())
        return error;

    QString type = errorElement.attribute("type");
    QString text;
    QString cond;
    QDomElement element = errorElement.firstChildElement();
    while(!element.isNull())
    {
        if(element.tagName() == "text")
            text = element.text();
        else if(element.namespaceURI() == ns_stanza)
        {
            cond = element.tagName();
        }        
        element = element.nextSiblingElement();
    }

    error.setConditionFromStr(cond);
    error.setTypeFromStr(type);
    error.setText(text);
    return error;
}

QAbstractSocket::SocketError QXmppStream::getSocketError()
{
    return m_socketError;
}

QXmppVCardManager& QXmppStream::getVCardManager()
{
    return m_vCardManager;
}

void QXmppStream::flushDataBuffer()
{
    m_dataBuffer.clear();
}
