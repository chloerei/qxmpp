#include <QDomElement>
#include <QXmlStreamWriter>

#include "QXmppConstants.h"
#include "QXmppIbbIq.h"

QXmppIbbOpenIq::QXmppIbbOpenIq() : QXmppIq(QXmppIq::Set), m_block_size(1024)
{

}

long QXmppIbbOpenIq::blockSize() const
{
    return m_block_size;
}

void QXmppIbbOpenIq::setBlockSize( long block_size )
{
    m_block_size = block_size;
}

QString QXmppIbbOpenIq::sid() const
{
   return  m_sid;
}

void QXmppIbbOpenIq::setSid( const QString &sid )
{
    m_sid = sid;
}

bool QXmppIbbOpenIq::isIbbOpenIq(const QDomElement &element)
{
    QDomElement openElement = element.firstChildElement("open");
    return openElement.namespaceURI() == ns_ibb;
}

void QXmppIbbOpenIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    setTypeFromStr(element.attribute("type"));

    QDomElement openElement = element.firstChildElement("open");
    m_sid = openElement.attribute( "sid" );
    m_block_size = openElement.attribute( "block-size" ).toLong();
}

void QXmppIbbOpenIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("open");
    writer->writeAttribute( "xmlns",ns_ibb);
    writer->writeAttribute( "sid",m_sid);
    writer->writeAttribute( "block-size",QString::number(m_block_size) );
    writer->writeEndElement();
}

QXmppIbbCloseIq::QXmppIbbCloseIq() : QXmppIq(QXmppIq::Set)
{

}

QString QXmppIbbCloseIq::sid() const
{
   return  m_sid;
}

void QXmppIbbCloseIq::setSid( const QString &sid )
{
    m_sid = sid;
}

bool QXmppIbbCloseIq::isIbbCloseIq(const QDomElement &element)
{
    QDomElement openElement = element.firstChildElement("close");
    return openElement.namespaceURI() == ns_ibb;
}

void QXmppIbbCloseIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    setTypeFromStr(element.attribute("type"));

    QDomElement openElement = element.firstChildElement("close");
    m_sid = openElement.attribute( "sid" );
}

void QXmppIbbCloseIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("close");
    writer->writeAttribute( "xmlns",ns_ibb);
    writer->writeAttribute( "sid",m_sid);
    writer->writeEndElement();
}


QXmppIbbDataIq::QXmppIbbDataIq() : QXmppIq( QXmppIq::Set ), m_seq(0)
{
}

quint16 QXmppIbbDataIq::sequence() const
{
    return m_seq;
}

void QXmppIbbDataIq::setSequence( quint16 seq )
{
    m_seq = seq;
}

QString QXmppIbbDataIq::sid() const
{
    return m_sid;
}

void QXmppIbbDataIq::setSid( const QString &sid )
{
    m_sid = sid;
}

QByteArray QXmppIbbDataIq::payload() const
{
    return m_payload;
}

void QXmppIbbDataIq::setPayload( const QByteArray &data )
{
    m_payload = data;
}

bool QXmppIbbDataIq::isIbbDataIq(const QDomElement &element)
{
    QDomElement dataElement = element.firstChildElement("data");
    return dataElement.namespaceURI() == ns_ibb;
}

void QXmppIbbDataIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    setTypeFromStr(element.attribute("type"));

    QDomElement dataElement = element.firstChildElement("data");
    m_sid = dataElement.attribute( "sid" );
    m_seq = dataElement.attribute( "seq" ).toLong();
    m_payload = QByteArray::fromBase64( dataElement.text().toLatin1() );
}

void QXmppIbbDataIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("data");
    writer->writeAttribute( "xmlns",ns_ibb);
    writer->writeAttribute( "sid",m_sid);
    writer->writeAttribute( "seq",QString::number(m_seq) );
    writer->writeCharacters( m_payload.toBase64() );
    writer->writeEndElement();
}
