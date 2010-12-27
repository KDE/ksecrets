/*
 * Copyright 2010, Valentin Rusu <kde@rusu.info>
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

#include <QtNetwork/QSslSocket>
#include <kdebug.h>

SyncServerJob::SyncServerJob(SyncDaemon* daemon, QSslSocket* socket) :
    _socket( socket ), _daemon( daemon )
{
    Q_ASSERT( socket != NULL );
    connect( socket, SIGNAL( stateChanged(QAbstractSocket::SocketState)) , SLOT( onSocketStateChanged(QAbstractSocket::SocketState) ) );
    connect( socket, SIGNAL( encrypted() ), SLOT( onSocketEncrypted() ) );
    connect( socket, SIGNAL( sslErrors(QList<QSslError>) ), SLOT( onSslErrors(QList<QSslError>) ) );
}

SyncServerJob::~SyncServerJob()
{
    delete _socket;
}

void SyncServerJob::start()
{
    kDebug() << "starting server sync job";
    _socket->startServerEncryption();
}

QString SyncServerJob::errorString() const
{
    return KJob::errorString();
}

void SyncServerJob::onSocketStateChanged( QAbstractSocket::SocketState state )
{
    kDebug() << "onSocketStateChanged " << state;
}

void SyncServerJob::onSslErrors( QList< QSslError > errorList )
{
    foreach( QSslError error, errorList ) {
        _daemon->createLogEntry( error.errorString() );
    }
}

void SyncServerJob::onSocketEncrypted()
{
    kDebug() << "onSocketEncrypted";
    if ( _socket->waitForReadyRead(-1) ) {
        
    }
}

#include "syncserverjob.moc"
