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

#include "syncserverjob.h"
#include "syncdaemon.h"
#include "syncprotocol.h"

#include <QtNetwork/QTcpSocket>
#include <QDebug>
#include <klocalizedstring.h>

#define SERVER_READ_TIMEOUT 60000

SyncServerJob::SyncServerJob(SyncDaemon* daemon, QTcpSocket* sshSocket) :
    _syncSocket( sshSocket ), _daemon( daemon ), _syncProtocol(0)
{
    Q_ASSERT( _daemon != NULL );
    Q_ASSERT( _syncSocket != NULL );
    connect( _syncSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)) , SLOT(onSocketStateChanged(QAbstractSocket::SocketState)) );
/*    connect( _syncSocket, SIGNAL(readyRead()), SLOT(onSocketReadyRead()) );
    connect( _syncSocket, SIGNAL(bytesWritten(qint64)), SLOT(onSocketBytesWritten(qint64)) );
    connect( _syncSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(onSocketError(QAbstractSocket::SocketError)) );*/
}

SyncServerJob::~SyncServerJob()
{
    delete _syncSocket;
}

void SyncServerJob::start()
{
    qDebug() << "starting server sync job";
    _syncProtocol = new SyncProtocolServer( _daemon );

    forever {
        // FIXME: read server timeouts from the global configuration
        if ( !_syncSocket->waitForReadyRead( SERVER_READ_TIMEOUT ) ) {
            setErrorText( i18n("Timeout while waiting for client request") );
            break;
        }
        
        // FIXME: adjust the size of this buffer
        char reqBuffer[1024];
        size_t reqLength = sizeof( reqBuffer );
        qint64 lineLength = _syncSocket->readLine( reqBuffer, reqLength );
        if ( -1 == lineLength ) {
            setErrorText( i18n("Error reading client request") );
            break;
        }
        
        QString reply;
        if ( !_syncProtocol->handleRequest( QString( reqBuffer ), reply ) ) {
            setErrorText( i18n("Protocol error : %1", _syncProtocol->errorString() ) );
            break;
        }
        
        if ( -1 == _syncSocket->write( qPrintable( reply ) ) ) {
            setErrorText( i18n("Error writing reply") );
            break;
        }
        
        if ( _syncProtocol->isDone() ) {
            setErrorText( i18n("Done synchronizing") );
            break;
        }
    }
    
    emitResult();
}

QString SyncServerJob::errorString() const
{
    return KJob::errorString();
}

void SyncServerJob::onSocketStateChanged( QAbstractSocket::SocketState state )
{
    qDebug() << "onSocketStateChanged " << state;
}

#include "syncserverjob.moc"
