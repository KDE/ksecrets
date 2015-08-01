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


#include "syncmodel.h"
#include "ksecretsynccfg.h"

#include <QVariant>
#include <QMap>
#include "klocalizedstring.h"

ComputerData::ComputerData( const QString& computerName ) : 
        _lastSyncStatus( SYNC_STATUS_NONE ) {
    QStringList nameParts = computerName.split(':');
    Q_ASSERT( nameParts.count() ==2 );
    Q_ASSERT( !nameParts[1].isEmpty() );
    _computerName = nameParts[0];
    _remotPort = nameParts[1].toInt();
}

class ComputerDataDisplayItem : public QStandardItem {
public:
    ComputerDataDisplayItem( ComputerData* data, int column) : _computerData( data ), _column( column ) {}

    virtual QVariant data( int role = Qt::UserRole +1 ) const {
        if ( role == Qt::DisplayRole )
            return QVariant( text() );
        return QStandardItem::data( role );
    }
    QString text() const
    {
        static const char *statusStrings[] = {
            "waiting for sync", //SYNC_STATUS_NONE,           /// computer data has just been loaded or created and no sync has been tried
            "sync pending", //SYNC_STATUS_PENDING,        /// a sync job was started on this item
            "sync done",    //SYNC_STATUS_DONE,           /// the last sync operation finished successfully
            "unreachable",  //SYNC_STATUS_UNREACHABLE,    /// the computer is not reachable
            "conflict",     //SYNC_STATUS_CONFLICTING,    /// the target computer has a conflicting item
            "error",  //SYNC_STATUS_ERROR           /// an error occurred during last sync operation
        };
        //TODO: implement text items caching below
        switch ( _column ) {
            case 0:
                return _computerData->_computerName;
            case 1:
                return statusStrings[ _computerData->_lastSyncStatus ];
            case 2:
                return _computerData->_lastSyncTime.toString();
            case 3:
                return _computerData->_remoteIP;
        }
        Q_ASSERT(0); // should never reach here - check list column count matches this method
        return "ERROR";
    }
    // TODO: add custom icons to reflect the computer sync status
private:
    ComputerData    *_computerData;
    int             _column;
};

class NoComputerDefinedItem : public QStandardItem {
public:
    NoComputerDefinedItem() {
        setText( i18n("No computer defined") );
        setData( i18n("Double click here to configure KSecretSync"), Qt::ToolTipRole );
    }
};

SyncModel::SyncModel() :
    QStandardItemModel()
{
    setColumnCount( 4 ); // computer name, sync status, last sync time, remote IP

    // TODO: add 5th column with status message for current computer
    
    QStringList horizontalLabels;
    horizontalLabels.append( i18n("Computer") );
    horizontalLabels.append( i18n("Sync status") );
    horizontalLabels.append( i18n("Last sync time") );
    horizontalLabels.append( i18n("Remote IP") );
    setHorizontalHeaderLabels( horizontalLabels );

    if ( KSecretSyncCfg::computerList().count() >0 ) {
        foreach( const QString &computerName, KSecretSyncCfg::computerList() ) {
            ComputerData *computerData = new ComputerData( computerName );
            QList< QStandardItem* > itemList;
            itemList.append( new ComputerDataDisplayItem( computerData, 0 ));
            itemList.append( new ComputerDataDisplayItem( computerData, 1 ));
            itemList.append( new ComputerDataDisplayItem( computerData, 2 ));
            itemList.append( new ComputerDataDisplayItem( computerData, 3 ));
            appendRow( itemList );
        }
    }
    else {
        appendRow( new NoComputerDefinedItem() );
    }
}

SyncModel::~SyncModel()
{

}

bool SyncModel::hasComputers() const
{
    bool hasComputers = false;
    if ( rowCount() > 0 ) {
    
        QStandardItem *firstItem = item( 0, 0 );
        if ( dynamic_cast< NoComputerDefinedItem* > ( firstItem ) ) {
            hasComputers = false; 
        }
        else {
            hasComputers = true;
        }
    }
    return hasComputers;
}

ComputerData* SyncModel::computerData(int index)
{
    return data( createIndex( index, 0 ) ).value< ComputerData* > (); 
}

#include "syncmodel.moc"
