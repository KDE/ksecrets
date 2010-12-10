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
#include <klocalizedstring.h>
#include <kicon.h>
#include <QTimer>
#include <QStandardItemModel>

SyncDaemon::SyncDaemon(QObject* parent) : 
    QObject(parent),
    _syncTimer(0)
{
    _computerList = new QStandardItemModel(0, 3, this);
    
    _syncTimer = new QTimer( this );
    connect( _syncTimer, SIGNAL(timeout()), SLOT(onSyncTimer()) );

    configChanged();
}

SyncDaemon::~SyncDaemon()
{
    delete _computerList;
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
    
    int computerCount = mainGroup.readEntry<int>(MAIN_COMPUTER_COUNT, 0);
    if ( computerCount ) {
        _hasComputers = true;
        // TODO: load computers here
    }
    else {
        _hasComputers = false;
        QStandardItem *noComputersItem = new QStandardItem( KIcon( QLatin1String("dialog-cancel") ), 
                                                            i18n("no computers defined") );
        noComputersItem->setWhatsThis( i18n("This item is present because no computers are defined. Click this item to open the settings module and to add a computer") );
        noComputersItem->setToolTip( i18n("Double click this item to open the configuration module") );
        _computerList->appendRow( noComputersItem );
    }
}

void SyncDaemon::onSyncTimer()
{
    startSync();
}

void SyncDaemon::startSync()
{
    kDebug() << "startSync";
    if (!_hasComputers)
        return;
    
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
