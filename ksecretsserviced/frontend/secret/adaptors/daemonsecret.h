/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
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

#ifndef DAEMON_SECRET_H
#define DAEMON_SECRET_H

#include "secretstruct.h"

#include <QtCore/QSharedDataPointer>
#include <QtCore/QByteArray>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusArgument>
#include <QtCrypto>

/**
 * This class represents secret (possibly encrypted) for transfer on the
 * D-Bus. Encryption and decryption is handled by the respective session.
 *
 * @remarks Objects of this class are implicitly shared
 */
class DaemonSecret
{
public:
    /**
     * Constructs an empty DaemonSecret.
     */
    DaemonSecret();

    /**
     * Copy constructor.
     */
    DaemonSecret(const DaemonSecret &other);
    
    DaemonSecret( const SecretStruct &secretStruct );

    /**
     * Destructor.
     */
    virtual ~DaemonSecret();

    /**
     * Set the session object D-Bus path.
     */
    void setSession(const QDBusObjectPath &session);

    /**
     * Get the session object D-Bus path.
     */
    const QDBusObjectPath &session() const;

    /**
     * Set the encryption parameters.
     */
    void setParameters(const QByteArray &parameters);

    /**
     * Get the encryption parameters.
     */
    const QByteArray &parameters() const;

    /**
     * Set the secret's encrypted value
     */
    void setValue(const QByteArray &value);

    /**
     * Get the secret's encrypted value
     */
    const QByteArray &value() const;
    
    /**
     * Set the secret's content type
     */
    void setContentType(const QString& contentType);
    
    /**
     * Get the secret's content type
     */
    const QString& contentType() const;
    
    SecretStruct secretStruct() const;

private:
    SecretStruct d;
};

// Q_DECLARE_METATYPE(DaemonSecret)
// 
// QDBusArgument &operator<<(QDBusArgument &argument, const DaemonSecret &secret);
// const QDBusArgument &operator>>(const QDBusArgument &argument, DaemonSecret &secret);

#endif
