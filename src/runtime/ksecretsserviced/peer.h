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

#ifndef DAEMON_PEER_H
#define DAEMON_PEER_H

#include <QtCore/QtGlobal>
#include <QtCore/QSharedDataPointer>

class PeerData;

/**
 * Representation of a daemon peer, which is typically a client application.
 * Note that the client application firstly connect to the dbus daemon which
 * forwards messages to ksecretsservice daemon using another connection.
 */
class Peer
{
public:
    /**
     * Construct an invalid (non-existent) Peer.
     */
    Peer();

    /**
     * Copy constructor.
     */
    Peer(const Peer &other);

    /**
     * Copy operator.
     */
    Peer &operator=(const Peer &other);

    /**
     * Construct a valid Peer representing the process with the given pid.
     */
    explicit Peer(uint pid);

    /**
     * Destructor.
     */
    ~Peer();

    /**
     * Get the running state of the peer process
     */
    bool isStillRunning() const;

    /**
     * Get the executable path of the running process
     */
    QString exePath() const;

    /**
     * Get the command line used to launch the running process
     */
    QByteArray cmdLine() const;

    /**
     * Check if the peer is valid. An invalid peer identifies test applications
     * or requests sent in-process.
     */
    bool isValid() const;

    /**
     * This method is used to get information about the current process and is intended
     * to be internally used by the ksecretsserviced daemon
     * @return reference to an internal Peer instance reflecting the current process
     */
    static const Peer& currentProcess();
    
private:
    /**
     * Helper method witch returns the /proc/pid path for the
     * peer process.
     *
     * @note Must only be called on valid peers.
     */
    QString procFileName() const;

    QSharedDataPointer<PeerData> d;
};

#endif // DAEMON_PEER_H

