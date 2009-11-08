#ifndef PACKET_H
#define PACKET_H
#include <QXmlStreamWriter>
#include <QVariant>
#include <QDomElement>
#include <QList>
/**
* Packets are serialized QVariants that map to XMLRPC types.
* @li int -> int
* @li double -> double,float
* @li string -> QString, QCString, char *, QColor, QFont
* @li datetime.iso8601 -> QDateTime, QTime, QDate
* @li array -> QList<QVariant>, QStringList
* @li struct -> QMap<QVariant>
* @li base64 -> QByteArray
*
* An example of how to use the XML Packets is as follows:
* @code
*	QList<QVariant> args;
*	args << m_db << m_username << m_password << dbQuery;
*	RequestMessage msg( "data.query", args );
*	ResponseMessage resp( SomeHttpDispatchObject( msg.xml() ) );
*	if( resp.isValid() )
*	{
*		int rows = resp.value().toMap()["widgets"].toInt();
*	}
*	else
*		qWarning("Error: %s", resp.error().latin1() );
* @endcode
* This example will construct invoke the data.query() method on the XMLRPC
* interface with the args.  It will then check for the response to see if
* it was valid.  If its valid the message contains a struct of values, one of
* which is "widgets" that is an integer.  The struct is converted to a QVariant
* map and we can convert it as such from the QVariant.  We can then get the
* QVariant for the "widgets" value and convert that to an integer.  If there was
* an error, the packet is marked invalid and will have an error message in it.  The
* error() message will return this message.  The struct in the value will be a valid
* error structure so it can be dealt with accordingly.
*/

namespace XMLRPC
{
/**
 * Base utility methods for working with XMLRPC messages.
@author Ian Reinhart Geiser <geiseri@kde.org>
*/
class MessageBase
{
public:
	MessageBase();
	virtual ~MessageBase();

	/**
	* Returns a human readable error that was recived from the server.
	*/
	QString error() const;

	/**
	 * Sets the human readable error message.
	 */
	virtual void setError( const QString &message ) const;

	/**
	* Returns if the current message is valid.
	*/
	bool isValid() const;

protected:
        virtual void marshall( QXmlStreamWriter *writer, const QVariant &val ) const;
	virtual QVariant demarshall( const QDomElement &elem ) const;


private:
	mutable QString m_message;
	mutable bool m_valid;
};

/**
* Creates an XMLRPC message that will call a method with a series of
* QVariants that will be converted to XMLRPC types.
@author Ian Reinhart Geiser <geiseri@kde.org>
*/
class RequestMessage : public MessageBase
{
public:
        RequestMessage( const QDomElement &element );
	/**
         * Creates an RequestMessage from an XML packet.
	 */
	RequestMessage( const QByteArray &xml );
	/**
	* Creates a method packet that will call method with a list of args.
	*/
	RequestMessage( const QByteArray &method, const QList<QVariant> &args );

	/**
	* Convinence ctor that creates a packet for a method with a single
	* argument.
	*/
	RequestMessage( const QByteArray &method, const QVariant &arg );

	virtual ~RequestMessage() {;}

	/**
	* Return the xml representation of the packet.
	*/
	QByteArray xml() const;
        void writeXml( QXmlStreamWriter *writer ) const;

	QByteArray method() const;
	QList< QVariant > args() const;


private:
	QByteArray m_method;
	QList<QVariant> m_args;

};

/**
* Decodes an XMLRPC message from a server into a set of QVariants.
@author Ian Reinhart Geiser <geiseri@kde.org>
*/
class ResponseMessage : public MessageBase
{
public:
        /**
        * Create a new recive packet with an xml packet
        */
        ResponseMessage( const QDomElement &element );

	/**
	* Create a new recive packet with an xml packet
	*/
	ResponseMessage( const QByteArray &xml );
	/**
	 * Create a new response message with data.
	 */
	ResponseMessage( const QList< QVariant >& theValue );

	/**
	 * Create a new response message with data. This is exactly like
	 * the above method save for it only takes a single QVariant.
	 */
	explicit ResponseMessage( const QVariant& theValue );

	virtual ~ResponseMessage() {;}
	/**
	* Returns the number of data elements.  Usually this is only
	* 1 element, but some services will return mutiple messages in
	* a single xmlrpc packet.
	*/
	int count() const;
	/**
	* Returns the native value for an xmlrpc message at an offset.
	* Usually this method can be called with no arguments.
	*/
	QVariant value( int idx = 0 ) const;

	/**
	* Return the xml representation of the packet.
	*/
	virtual QByteArray xml() const;
        void writeXml( QXmlStreamWriter *writer ) const;


	QList< QVariant > values() const;
protected:
	void setValues( const QList<QVariant> va2ls);

private:
	QList<QVariant> m_values;
};

class FaultMessage : public ResponseMessage
{
public:
	FaultMessage( int code, const QString &message );
	QByteArray xml() const;
        void writeXml( QXmlStreamWriter *writer ) const;

};
}
#endif
