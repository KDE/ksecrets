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

#include "syncdaemon.h"
#include "syncjob.h"
#include "computersyncjob.h"
#include "configconstants.h"
#include "computerdata.h"

#include <kglobal.h>
#include <kdebug.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <QTimer>
#include <QStandardItemModel>

SyncDaemon::SyncDaemon(QObject* parent) : 
    QObject(parent),
    _syncTimer(0)
{
    
    _syncTimer = new QTimer( this );
    connect( _syncTimer, SIGNAL(timeout()), SLOT(onSyncTimer()) );

}

SyncDaemon::~SyncDaemon()
{

}

void SyncDaemon::configChanged()
{
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup mainGroup( config, "main" );

    Q_ASSERT(_syncTimer!=0);
    _syncTimer->stop();
    int syncInterval = mainGroup.readEntry<int>(MAIN_SYNC_INTERVAL, MIN_SYNC_INTERVAL );
    _syncTimer->setInterval( syncInterval * 60000 );
    _syncTimer->start();
}

void SyncDaemon::onSyncTimer()
{
    startSync();
}

void SyncDaemon::startSync()
{
    kDebug() << "startSync";
    if ( _computerList->rowCount() == 0 ) {
        return;
    }
    
    SyncJob *syncJob = new SyncJob;
    for (int r =0; r < _computerList->rowCount(); ++r ) {
        QStandardItem *computerItem = _computerList->item(r);
        Q_ASSERT(computerItem != 0);
        new ComputerSyncJob( syncJob, _computerData[ computerItem->text() ], this );
    }
    syncJob->start();
}

void SyncDaemon::createLogEntry(const QString& )
{
    // TODO: implement this by forwarding message to the logger
}
