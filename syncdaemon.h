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

#ifndef SYNCDAEMON_H
#define SYNCDAEMON_H

#include "computerdata.h"
#include "synclogger.h"

#include <QObject>
#include <QMap>

class QTimer;
class QStandardItemModel;


class SyncDaemon : public QObject, public SyncLogger
{
    Q_OBJECT
public:
    explicit SyncDaemon(QObject* parent = 0);
    virtual ~SyncDaemon();

    QStandardItemModel* computerList() const { return _computerList; }
    bool hasComputers() const { return _hasComputers; }
    
protected Q_SLOTS:
    void onSyncTimer();
    void configChanged();
    
protected:
    void startSync();
    virtual void createLogEntry( const QString& );
    
private:
    bool                        _syncEnabled;
    QTimer                      *_syncTimer;
    QStandardItemModel          *_computerList;
    QMap<QString, ComputerData> _computerData;
    bool                        _hasComputers;
};

#endif // SYNCDAEMON_H
