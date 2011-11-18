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

#include "collectionadaptor.h"
#include "../collection.h"

namespace orgFreedesktopSecret
{

CollectionAdaptor::CollectionAdaptor(Collection *collection)
    : QDBusAbstractAdaptor(collection), m_collection(collection)
{
    Q_ASSERT(collection);

    connect(m_collection, SIGNAL(itemCreated(QDBusObjectPath)),
            SIGNAL(ItemCreated(QDBusObjectPath)));
    connect(m_collection, SIGNAL(itemDeleted(QDBusObjectPath)),
            SIGNAL(ItemDeleted(QDBusObjectPath)));
    connect(m_collection, SIGNAL(itemChanged(QDBusObjectPath)),
            SIGNAL(ItemChanged(QDBusObjectPath)));
}

const QList<QDBusObjectPath> &CollectionAdaptor::items() const
{
    return m_collection->items();
}

void CollectionAdaptor::setLabel(const QString &label)
{
    m_collection->setLabel(label);
}

QString CollectionAdaptor::label() const
{
    return m_collection->label();
}

bool CollectionAdaptor::locked() const
{
    return m_collection->locked();
}

qulonglong CollectionAdaptor::created() const
{
    return m_collection->created();
}

qulonglong CollectionAdaptor::modified() const
{
    return m_collection->modified();
}

QDBusObjectPath CollectionAdaptor::Delete()
{
    return m_collection->deleteCollection();
}

QList<QDBusObjectPath> CollectionAdaptor::SearchItems(const StringStringMap &attributes)
{
    return m_collection->searchItems(attributes);
}

QDBusObjectPath CollectionAdaptor::CreateItem(const QMap<QString, QVariant> &properties,
        const SecretStruct &secret, bool replace,
        QDBusObjectPath &prompt)
{
    return m_collection->createItem(properties, secret, replace, prompt);
}

QDBusObjectPath CollectionAdaptor::ChangePassword()
{
    return m_collection->changePassword();
}

}

#include "collectionadaptor.moc"
