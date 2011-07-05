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

#include "daemonsecret.h"

#include <QtCore/QSharedData>

DaemonSecret::DaemonSecret()
{
}

DaemonSecret::DaemonSecret(const DaemonSecret &other) : d(other.d)
{
}

DaemonSecret::DaemonSecret(const SecretStruct& secretStruct) :
    d( secretStruct )
{

}

DaemonSecret::~DaemonSecret()
{
}

void DaemonSecret::setSession(const QDBusObjectPath &session)
{
    d.m_session = session;
}

const QDBusObjectPath &DaemonSecret::session() const
{
    return d.m_session;
}

void DaemonSecret::setParameters(const QByteArray &parameters)
{
    d.m_parameters = parameters;
}

const QByteArray &DaemonSecret::parameters() const
{
    return d.m_parameters;
}

void DaemonSecret::setValue(const QByteArray &value)
{
    // TODO: encryption
    d.m_value = value;
}

const QByteArray &DaemonSecret::value() const
{
    // TODO: decryption
    return d.m_value;
}

void DaemonSecret::setContentType(const QString& contentType)
{
    d.m_contentType = contentType;
}

SecretStruct DaemonSecret::secretStruct() const
{
    return d;
}

const QString& DaemonSecret::contentType() const
{
    return d.m_contentType;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DaemonSecret &secret)
{
    argument.beginStructure();
    argument << secret.session() << secret.parameters() << secret.value() << secret.contentType();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DaemonSecret &secret)
{
    QDBusObjectPath session;
    QByteArray parameters;
    QByteArray value;
    QString contentType;

    argument.beginStructure();
    argument >> session >> parameters >> value >> contentType;
    argument.endStructure();

    secret.setSession(session);
    secret.setParameters(parameters);
    secret.setValue(value);
    secret.setContentType(contentType);

    return argument;
}
