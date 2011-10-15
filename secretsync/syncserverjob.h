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

#ifndef SYNCSERVERJOB_H
#define SYNCSERVERJOB_H

#include <kjob.h>
#include <QtNetwork/QSslError>
#include <QtNetwork/QAbstractSocket>

class SyncProtocolServer;
class QTcpSocket;
class SyncDaemon;

class SyncServerJob : public KJob
{
    Q_OBJECT
public:
    SyncServerJob( SyncDaemon* daemon, QTcpSocket* sshSocket);
    virtual ~SyncServerJob();
    
    virtual void start();
    virtual QString errorString() const;
    
protected Q_SLOTS:
    void onSocketStateChanged(QAbstractSocket::SocketState);
    
private:
    QTcpSocket              *_syncSocket;
    SyncDaemon              *_daemon;
    SyncProtocolServer      *_syncProtocol;
};

#endif // SYNCSERVERJOB_H
