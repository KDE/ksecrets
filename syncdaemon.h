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

#include "syncmodel.h"
#include "synclogger.h"

#include <QtNetwork/QTcpServer>
#include <QMap>

class QTimer;

class SyncDaemon : public QTcpServer, public SyncLogger
{
    Q_OBJECT
public:
    enum ListeningState {
        LISTENING_NOT_INITIALIZED,
        LISTENING_READY,
        LISTENING_ERROR
    };
    
    SyncDaemon(QObject* parent, SyncModel* model);
    virtual ~SyncDaemon();

    virtual void createLogEntry( const QString& );
   
protected Q_SLOTS:
    void onSyncTimer();
    void onNewConnection();
    
protected:
    void startSyncing();
    void doSync();
    void startListening();
    static void qMsgHandler( QtMsgType type, const char* msg );
    
private:
    static SyncDaemon  *_instance;
    ListeningState      _listeningState;
    bool        _syncEnabled;
    QTimer      *_syncTimer;
    SyncModel   *_model;
};

#endif // SYNCDAEMON_H
