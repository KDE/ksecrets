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

#include "syncdaemon.h"
#include "syncjob.h"
#include "computersyncjob.h"
#include "configconstants.h"
#include "ksecretsynccfg.h"
#include "syncserverjob.h"

#include <kglobal.h>
#include <QDebug>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <kicon.h>
#include <QTimer>
#include <QStandardItemModel>
#include <QtNetwork/QSslSocket>
#include <stdio.h>
#include <stdlib.h>
//#include <libssh2.h>

SyncDaemon* SyncDaemon::_instance = 0;

SyncDaemon::SyncDaemon(QObject* parent, SyncModel* model) : 
    QTcpServer(parent),
    _listeningState( LISTENING_NOT_INITIALIZED ),
    _syncTimer(0),
    _model( model )
{
    Q_ASSERT( model != 0 );
    Q_ASSERT( _instance == 0);
    _instance = this;

    qInstallMsgHandler( qMsgHandler );

    startSyncing();
    startListening();
}

SyncDaemon::~SyncDaemon()
{
    delete _syncTimer; // NOTE: QTimer destructor calls stop() if needed
}

void SyncDaemon::startSyncing()
{
    _syncTimer = new QTimer( this );
    connect( _syncTimer, SIGNAL(timeout()), SLOT(onSyncTimer()) );

    if ( KSecretSyncCfg::enableSync() ){
        int syncInterval = KSecretSyncCfg::syncInterval() * 60000; // the sync interval is configured in minutes
        createLogEntry( i18n("SyncDaemon started sync; syncInterval = %1", syncInterval) );
        _syncTimer->setInterval( syncInterval );
        _syncTimer->start();
    }
    else {
        createLogEntry( i18n("SyncDaemon sync is currently disabled") );
    }
}

void SyncDaemon::onSyncTimer()
{
    doSync();
}

void SyncDaemon::doSync()
{
    // FIXME: should we create a log entry instead? pitfall: create too many log entries
    qDebug() << "startSync";
    if (!_model->hasComputers())
        return;

    SyncJob *syncJob = new SyncJob;
    for (int r =0; r < _model->rowCount(); ++r ) {
        new ComputerSyncJob( syncJob, _model->computerData( r ), this );
    }

    syncJob->start();
}

void SyncDaemon::startListening()
{
    connect(this, SIGNAL(newConnection()), SLOT(onNewConnection()) );
    
    // TODO: implement listening host address specifying here, e.g. allow user specify which card to bind to
    if ( listen( QHostAddress::Any, KSecretSyncCfg::listeningPort() ) ) {
        createLogEntry( i18n("started listening on the network") );
        _listeningState = LISTENING_READY;
    }
    else {
        createLogEntry( i18n("start listening failed : %1", errorString() ) );
        _listeningState = LISTENING_ERROR;
    }
}

void SyncDaemon::onNewConnection()
{
    QTcpSocket *connection = nextPendingConnection();
    QString logEntry( i18n("Incoming connection from %1", connection->peerAddress().toString() ) );
    createLogEntry( logEntry );
    SyncServerJob* serverJob = new SyncServerJob( this, connection );
    serverJob->start();
}

void SyncDaemon::qMsgHandler(QtMsgType type, const char* msg)
{
    if ( _instance ) {
        QString message;
        switch ( type ) {
            case QtDebugMsg:
                message = "QtDebug";
                break;
            case QtWarningMsg:
                message = "QtWarning";
                break;
            case QtCriticalMsg:
                message = "QtCritical";
                break;
            case QtFatalMsg:
                message = "QtFatal";
                break;
            default:
                message = "QtMsg UNKNOWN";
        }
        message += ": ";
        message += msg;
//        _instance->createLogEntry( message );
        fprintf( stderr, "%s\n", qPrintable( message ) );
    }
}

void SyncDaemon::createLogEntry(const QString& message )
{
    qDebug() << message;
    // TODO: implement this by forwarding message to the logger
}

#include "syncdaemon.moc"
