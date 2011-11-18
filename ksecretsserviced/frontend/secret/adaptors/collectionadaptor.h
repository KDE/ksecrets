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

#ifndef ORG_FREEDESKTOP_SECRET_COLLECTIONADAPTOR_H
#define ORG_FREEDESKTOP_SECRET_COLLECTIONADAPTOR_H

#include "dbustypes.h"

#include <QtDBus/QDBusAbstractAdaptor>

class Collection;

namespace orgFreedesktopSecret
{

/**
 * D-Bus adaptor class for Collection objects.
 */
class CollectionAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Secret.Collection")
    Q_PROPERTY(QList<QDBusObjectPath> Items READ items)
    Q_PROPERTY(QString Label READ label WRITE setLabel)
    Q_PROPERTY(QString Locked READ locked)
    Q_PROPERTY(qulonglong Created READ created)
    Q_PROPERTY(qulonglong Modified READ modified)

public:
    /**
     * Constructor.
     *
     * @param collection collection object to attach the adaptor to
     */
    CollectionAdaptor(Collection *collection);

    const QList<QDBusObjectPath> &items() const;

    void setLabel(const QString &label);

    QString label() const;

    bool locked() const;

    qulonglong created() const;

    qulonglong modified() const;
    
public Q_SLOTS:
    QDBusObjectPath Delete();

    QList<QDBusObjectPath> SearchItems(const StringStringMap &attributes);

    QDBusObjectPath CreateItem(const QMap<QString, QVariant> &properties,
                               const SecretStruct &secret, bool replace,
                               QDBusObjectPath &prompt);
    /**
     * Request password change on the current collection
     * @note this metod is KDE specific
     * @return the DBus path to the prompt used to change the password of this collection
     */
    QDBusObjectPath ChangePassword();

Q_SIGNALS:
    void ItemCreated(const QDBusObjectPath &item);

    void ItemDeleted(const QDBusObjectPath &item);

    void ItemChanged(const QDBusObjectPath &item);

private:
    Collection *m_collection;
};

}

#endif
