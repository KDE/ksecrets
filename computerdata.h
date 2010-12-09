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

#ifndef COMPUTERDATA_H
#define COMPUTERDATA_H

#include <QDateTime>
#include <QStringList>

/**
 * Information structure used to manage synchronization with a given computer
 * KSecretSync use one ComputerData for each known computer
 */
struct ComputerData
{
    enum SyncStatus {
        SYNC_STATUS_NONE,           /// computer data has just been loaded or created and no sync has been tried
        SYNC_STATUS_PENDING,        /// a sync job was started on this item
        SYNC_STATUS_DONE,           /// the last sync operation finished successfully
        SYNC_STATUS_UNREACHABLE,    /// the computer is not reachable
        SYNC_STATUS_CONFLICTING,    /// the target computer has a conflicting item
        SYNC_STATUS_ERROR           /// an error occurred during last sync operation
    };
    
    QString     _computerName;
    QString     _computerAddress;
    QTime       _lastSyncTime;
    SyncStatus  _lastSyncStatus;
    QString     _lastSyncError;
    QStringList _conflictingItems;
};

#endif // COMPUTERDATA_H
