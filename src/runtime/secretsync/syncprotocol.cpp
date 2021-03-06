/*
 * Copyright 2010, Valentin Rusu <valir@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "syncprotocol.h"
#include "ksecretsynccfg.h"
#include "synclogger.h"
//#include <ksecretsservicedbustypes.h>
#include <ksecretsservice/ksecretsservicecollection.h>
// #include "service_interface.h"
// #include "session_interface.h"

#include <QDebug>
#include <klocalizedstring.h>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnectionInterface>
#include <QMap>

const int SyncProtocol::VERSION = 1;

SyncProtocol::SyncProtocol( SyncLogger *logger ) :
    _state( STATE_INIT ),
    _phase(0),
    _logger( logger )
{

}

SyncProtocol::~SyncProtocol()
{

}

void SyncProtocol::changeState(SyncProtocol::State state)
{
    if ( state != _state ) {
        Phase* newPhase = 0;
        switch ( state ) {
            case STATE_HELLO:
                newPhase = new PhaseHello(this);
                break;
            case STATE_LIST_ITEMS:
                newPhase = new PhaseListItems(this);
                break;
            case STATE_DONE:
                newPhase = 0;
                disconnectFromSecretService();
                break;
            default:
                Q_ASSERT(0);
                break;
        }
        if ( newPhase ) {
            qDebug() << "changing state from " << (_phase ? _phase->name() : "NONE") << " to " << newPhase->name();
            delete _phase;
            _phase = newPhase;
        }
        _state = state;
    }
}

bool SyncProtocol::tryHandlingReply(const QString& reply)
{
    State newState = phase()->handleReply( reply );
    changeState( newState );
    return newState != STATE_ERROR;
}

bool SyncProtocol::tryHandlingRequest(const QString& request, QString &reply)
{
    Q_ASSERT( _state != STATE_DONE ); // if this was asserted, then this instance of the protocol is no longer valid
    
    State newState = phase()->handleRequest( request, reply );
    Q_ASSERT( !reply.isEmpty() ); // forgot to set reply in Phase ?
    changeState( newState );
    return newState != STATE_ERROR; 
}

void SyncProtocol::createLogEntry(const QString& entry)
{
    _logger->createLogEntry( entry );
}



bool SyncProtocol::connectToSecretService()
{
    // TODO: implement this
    return false;
}

void SyncProtocol::disconnectFromSecretService()
{
    // TODO: implement this
}

const char* SyncProtocol::Phase::name() const
{
    const char* names[ SyncProtocol::STATE_COUNT ] = {
        "STATE_INIT",
        "STATE_HELLO",
        "STATE_LIST_ITEMS"
    };
    return names[ _state ];
}

SyncProtocol::State SyncProtocol::Phase::errorHandlingRequest(const QString& request)
{
    qDebug() << "ERROR handling request " << request;
    return STATE_ERROR;
}

SyncProtocol::State SyncProtocol::Phase::handleReply(const QString& reply)
{
    SyncProtocol::State result = SyncProtocol::STATE_ERROR;
    // reply MUST have the form :
    // NNN Message
    // where NNN is an status code using three digits
    if ( reply.length() > 4) {
        QString statusCodeString = reply.left(3);
        bool statusCodeOk = false;
        _replyStatusCode = statusCodeString.toInt( &statusCodeOk );
        _replyMessage = reply.mid(4); // skip the space between status code and the message
        if ( !statusCodeOk ) {
            result = SyncProtocol::STATE_ERROR;
            onInvalidReply();
        }
        else {
            if ( ( _replyStatusCode >= 200 ) && ( _replyStatusCode < 300 ) ) {
                result = handleOkReply( _replyStatusCode, _replyMessage );
            }
            else {
                result = handleErrorReply( _replyStatusCode, _replyMessage );
            }
        }
    }
    else {
        onInvalidReply();
    }
    
    return result;
}

SyncProtocol::State SyncProtocol::Phase::handleErrorReply(int statusCode, const QString& message)
{
    return SyncProtocol::STATE_ERROR;
}

QString SyncProtocol::PhaseHello::request()
{
    return QString("HELLO ksecretsync %1").arg( SyncProtocol::VERSION );
}

SyncProtocol::State SyncProtocol::PhaseHello::handleRequest(const QString& req, QString& response)
{
    if ( req == request() ) {
        response = "200 OK";
        return STATE_LIST_ITEMS;
    }
    response = "500 VERSION NOT SUPPORTED";
    qDebug() << "ERROR: expecting " << request();
    return SyncProtocol::Phase::errorHandlingRequest( req );
}

SyncProtocol::State SyncProtocol::PhaseHello::handleOkReply(int code, const QString& message)
{
    return SyncProtocol::STATE_LIST_ITEMS;
}

const char* SyncProtocol::PhaseListItems::REQUEST = "LIST ITEMS";
const char* SyncProtocol::PhaseListItems::NO_MORE_ITEMS = "201 NO MORE ITEMS";

/**
 * The first time the service is run, lastSyncTime will be 0 so all items wille be synchronized
 */
QString SyncProtocol::PhaseListItems::request()
{
    uint lastSyncTime = KSecretSyncCfg::self()->lastSyncTime();
    return QString("%1 %2").arg( REQUEST ).arg( lastSyncTime );
}

SyncProtocol::State SyncProtocol::PhaseListItems::handleRequest(const QString& req, QString& response)
{
    if ( req.startsWith( REQUEST ) ) {
        QString timeStampStr = req.mid( QString(REQUEST).length() ).trimmed();
        uint timeStamp = timeStampStr.toUInt();
        
        // now connect to KSecretService daemon and list all the items to get all items path
        if ( !_protocol->connectToSecretService() ) {
            response = "501 Internal Error (dbus)";
            return SyncProtocol::Phase::errorHandlingRequest( req );
        }
        
        // list = Service::searchItems
        QList<QDBusObjectPath> lockedItems;
        QList<QDBusObjectPath> unlockedItems;
        QMap< QString, QString > searchEmptyAttrs; // this will stay empty to get all items
        // FIXME: the prototype of this method was changed
        //unlockedItems = _protocol->_secretsService->searchItems( searchEmptyAttrs, lockedItems );
        
        if ( unlockedItems.length() == 0 ) {
            response = NO_MORE_ITEMS;
            return STATE_DONE;
        }
        
        // itemList = Service::getSecrets(list)
        // foreach in itemList 
    }
    return errorHandlingRequest( req );
}

SyncProtocol::State SyncProtocol::PhaseListItems::handleOkReply(int statusCode, const QString& message)
{
    SyncProtocol::State result = STATE_ERROR;
    switch ( statusCode ) {
        case 201: 
            // NO_MORE_ITEMS
            result = STATE_DONE;
            break;
    }
    return result;
}


SyncProtocolServer::SyncProtocolServer( SyncLogger *logger ) : SyncProtocol( logger )
{
    changeState( STATE_HELLO );
}

bool SyncProtocolServer::handleRequest(const QString& request, QString& response)
{
    return tryHandlingRequest( request, response );
}

SyncProtocolClient::SyncProtocolClient( SyncLogger *logger ) : SyncProtocol( logger )
{
    changeState( STATE_HELLO );
}

QString SyncProtocolClient::nextRequest()
{
    return phase()->request();
}

bool SyncProtocolClient::handleReply( const QString& reply )
{
    return tryHandlingReply( reply );
}

