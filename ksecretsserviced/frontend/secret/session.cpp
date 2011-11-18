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

#include "session.h"
#include "adaptors/sessionadaptor.h"
#include "service.h"
#include "peer.h"

#include "../lib/secrettool.h"

#include <QtDBus/QDBusConnection>
#include <QtCore/QRegExp>

Session::Session(Service *parent, SecretCodec *codec)
    : QObject(parent),
      m_objectPath(parent->objectPath().path() + "/session/" + createId()),
      m_secretCodec(codec)
{
    // register on the bus
    new orgFreedesktopSecret::SessionAdaptor(this);
    QDBusConnection::sessionBus().registerObject(m_objectPath.path(), this);
}

Session::~Session()
{
    delete m_secretCodec;
}

const QDBusObjectPath &Session::objectPath() const
{
    return m_objectPath;
}

Session *Session::create(const QString &algorithm, const QVariant &input,
                         QVariant &output, const Peer &peer, Service *parent)
{
    Session *session = 0;

    if(algorithm == "plain") {
        session = new Session(parent, 0);
        output.setValue(QString(""));
    }
    else {
        SecretCodec *codec = new SecretCodec;
        if ( codec->initServer( algorithm, input, output ) ) {
            session = new Session(parent, codec);
        }
        else {
            delete codec;
        }
    }

    if ( session ) {
        session->m_peer = peer;
    }
    
    return session;
}

bool Session::encrypt(const QCA::SecureArray &value, QCA::SecureArray &encrypted, QByteArray &encryptedParams)
{
    bool result = true;
    if ( m_secretCodec ) {
        result = m_secretCodec->encryptServer( value, encrypted, encryptedParams );
    }
    else {
        encrypted = value;
    }
    return result;
}

bool Session::decrypt(const QCA::SecureArray &encrypted, const QByteArray &encryptedParams, QCA::SecureArray &value)
{
    bool result = true;
    if ( m_secretCodec ) {
        result = m_secretCodec->decryptServer( encrypted, encryptedParams, value );
    }
    else {
        value = encrypted;
    }
    return result;
}

void Session::close()
{
    deleteLater();
}

const Peer &Session::peer() const
{
    return m_peer;
}

#include "session.moc"
