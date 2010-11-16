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

#include <QtCore/QSharedDataPointer>
#include <QtCore/QByteArray>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusArgument>
#include <QtCrypto/QtCrypto>

/**
 * This class represents secret (possibly encrypted) for transfer on the
 * D-Bus. Encryption and decryption is handled by the respective session.
 *
 * @remarks Objects of this class are implicitly shared
 */
class Secret
{
    // TODO: this should actually be inside secret.cpp but I'm getting strange
    //       errors if I put it there. investigate.
    class SecretData : public QSharedData
    {
    public:
        SecretData() {}
        SecretData(const SecretData &other)
            : QSharedData(other), m_session(other.m_session), m_parameters(other.m_parameters),
              m_value(other.m_value) {}
        ~SecretData() {}
        QDBusObjectPath m_session;
        QByteArray m_parameters;
        QByteArray m_value;
    };

public:
    /**
     * Constructs an empty Secret.
     */
    Secret();

    /**
     * Copy constructor.
     */
    Secret(const Secret &other);

    /**
     * Destructor.
     */
    ~Secret();

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

private:
    class SecretData;
    QSharedDataPointer<SecretData> d;
};

Q_DECLARE_METATYPE(Secret)

QDBusArgument &operator<<(QDBusArgument &argument, const Secret &secret);
const QDBusArgument &operator>>(const QDBusArgument &argument, Secret &secret);

#endif
