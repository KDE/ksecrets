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

#include "serviceadaptor.h"
#include "dbustypes.h"
#include "../service.h"

namespace orgFreedesktopSecret
{

ServiceAdaptor::ServiceAdaptor(Service *service)
    : QDBusAbstractAdaptor(service), m_service(service)
{
    Q_ASSERT(service);

    // register all types needed for the D-Bus interface
    registerDBusTypes();

    connect(service, SIGNAL(collectionCreated(QDBusObjectPath)),
            SIGNAL(CollectionCreated(QDBusObjectPath)));
    connect(service, SIGNAL(collectionDeleted(QDBusObjectPath)),
            SIGNAL(CollectionDeleted(QDBusObjectPath)));
    connect(service, SIGNAL(collectionChanged(QDBusObjectPath)),
            SIGNAL(CollectionChanged(QDBusObjectPath)));
}

const QList<QDBusObjectPath> &ServiceAdaptor::collections() const
{
    return m_service->collections();
}

QDBusVariant ServiceAdaptor::OpenSession(const QString &algorithm, const QDBusVariant &input,
        QDBusObjectPath &result)
{
    return QDBusVariant(m_service->openSession(algorithm, input.variant(), result));
}

QDBusObjectPath ServiceAdaptor::CreateCollection(const QMap<QString, QVariant> &properties,
						 const QString& alias,
						 QDBusObjectPath &prompt)
{
    return m_service->createCollection(properties, alias, prompt);
}

QList<QDBusObjectPath> ServiceAdaptor::SearchItems(const StringStringMap &attributes,
        QList<QDBusObjectPath> &locked)
{
    return m_service->searchItems(attributes, locked);
}

QList<QDBusObjectPath> ServiceAdaptor::Unlock(const QList<QDBusObjectPath> &objects,
        QDBusObjectPath &prompt)
{
    return m_service->unlock(objects, prompt);
}

QList<QDBusObjectPath> ServiceAdaptor::Lock(const QList<QDBusObjectPath> &objects,
        QDBusObjectPath &prompt)
{
    return m_service->lock(objects, prompt);
}

ObjectPathSecretMap ServiceAdaptor::GetSecrets(const QList<QDBusObjectPath> &items,
        const QDBusObjectPath &session)
{
    return m_service->getSecrets(items, session);
}

}

#include "serviceadaptor.moc"
