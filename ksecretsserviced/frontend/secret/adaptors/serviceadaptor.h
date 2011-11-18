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

#ifndef ORG_FREEDESKTOP_SECRET_SERVICEADAPTOR_H
#define ORG_FREEDESKTOP_SECRET_SERVICEADAPTOR_H

#include "dbustypes.h"
#include "daemonsecret.h"

#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusObjectPath>

class Service;

namespace orgFreedesktopSecret
{

/**
 * D-Bus adaptor class for Service objects.
 */
class ServiceAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Secret.Service")
    Q_PROPERTY(QList<QDBusObjectPath> Collections READ collections)

public:
    /**
     * Constructor.
     *
     * @param service service object to attach the adaptor to
     */
    ServiceAdaptor(Service *service);

    const QList<QDBusObjectPath> &collections() const;

public Q_SLOTS:
    QDBusVariant OpenSession(const QString &algorithm, const QDBusVariant &input,
                             QDBusObjectPath &result);

    QDBusObjectPath CreateCollection(const QMap<QString, QVariant> &properties,
				     const QString& alias,
                                     QDBusObjectPath &prompt);

    QList<QDBusObjectPath> SearchItems(const StringStringMap &attributes,
                                       QList<QDBusObjectPath> &locked);

    QList<QDBusObjectPath> Unlock(const QList<QDBusObjectPath> &objects, QDBusObjectPath &prompt);

    QList<QDBusObjectPath> Lock(const QList<QDBusObjectPath> &objects, QDBusObjectPath &prompt);

    ObjectPathSecretMap GetSecrets(const QList<QDBusObjectPath> &items,
                                   const QDBusObjectPath &session);

Q_SIGNALS:
    void CollectionCreated(const QDBusObjectPath &collection);

    void CollectionDeleted(const QDBusObjectPath &collection);

    void CollectionChanged(const QDBusObjectPath &collection);

private:
    Service *m_service;
};

}

#endif
