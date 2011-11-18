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

#ifndef KSECRETOBJECT_H
#define KSECRETOBJECT_H

#include <QtDBus/QDBusContext>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnectionInterface>
#include <peer.h>


/**
 * This is a base class for all the KSecretService d-bus exposed classes
 * and is used to get d-bus peer context information, used mainly for ACL
 * handling
 */
template <class D>
class KSecretObject
{
public:
    /**
     * The default constructor of this class does nothing
     */
    KSecretObject() {};
    virtual ~KSecretObject() {}

    /**
     * Get information about the calling peer
     * @return peer information or an invalid peer if the D-Bus context is not available,
     *         e.g. when the service method was not called across D-Bus.
     */
    Peer getCallingPeer() const
    {
        // NOTE: this class inheritor must also inherit the dbus object class from QDBusContext
        const D *ctxThis = dynamic_cast< const D* >( this );
        if( ctxThis->calledFromDBus()) {
            QString msgService = ctxThis->message().service();
            uint pid = ctxThis->connection().interface()->servicePid(msgService);
            return Peer(pid);
            // TODO: add syslog entry ?
        } else {
            return Peer::currentProcess();
        }
    }
};


#endif // KSECRETOBJECT_H
