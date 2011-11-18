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

#ifndef DAEMON_SESSION_H
#define DAEMON_SESSION_H

#include "adaptors/daemonsecret.h"

#include <peer.h>

#include <QtCore/QObject>
#include <QtCrypto/QtCrypto>

#include "ksecretobject.h"
#include <ksecretsservicecodec.h>

using namespace KSecretsService;

class Service;
class Peer;

/**
 * Represents an open session of a client on the D-Bus implementing the
 * org.freedesktop.Secret.Session interface.
 *
 * @todo stub implementation, currently only supports plain (no encryption)
 */
class Session : public QObject, protected QDBusContext, public KSecretObject<Session>
{
    Q_OBJECT

private:
    /**
     * Constructor.
     *
     * @param parent Parent Service object
     */
    Session(Service *parent, SecretCodec *codec);

public:
    /**
     * Destructor.
     */
    ~Session();

    /**
     * Get the session's object path.
     */
    const QDBusObjectPath &objectPath() const;

    /**
     * Try to create a (possibly encrypted) session for the use of transferring
     * secrets.
     *
     * @param algorithm the negotiation/encryption algorithm to use
     * @param input negotiation algorithm input
     * @param output negotiation algorithm output
     * @param parent Parent Service object
     * @return a session object if negotiation was successful, 0 if the algorithm isn't
     *         supported or an error occurred
     */
    static Session *create(const QString &algorithm, const QVariant &input,
                           QVariant &output, const Peer &peer, Service *parent);

    /**
     * Encrypt a secret value using this session.
     *
     * @param value value to encrypt
     * @param encrypted the secret encrypted for transport
     * @return true if encrypting succeeds
     */
    bool encrypt(const QCA::SecureArray &value, QCA::SecureArray &encrypted, QByteArray &encryptedParams);

    /**
     * Decrypt a secret value using this session.
     *
     * @param secret DaemonSecret received on the D-Bus
     * @param value unencryped value
     * @return true if decrypting succeeds
     */
    bool decrypt(const QCA::SecureArray &encrypted, const QByteArray &encryptedParams, QCA::SecureArray &value);

    /**
     * Close this session.
     */
    void close();

    /**
     * Get the session's D-Bus peer (the client requesting the session).
     * If peer() == NULL, then this session was not created across D-Bus.
     * That can happen only during tests.
     */
    const Peer &peer() const;

private:
    QDBusObjectPath m_objectPath;
    Peer m_peer; // Information about the application that requested this session

    SecretCodec  *m_secretCodec;
};

#endif
