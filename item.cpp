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

#include "item.h"
#include "collection.h"
#include "dbus/itemadaptor.h"
#include "secret.h"

#include <backend/backenditem.h>

#include <QtDBus/QDBusConnection>

Item::Item(BackendItem *item, Collection *collection)
 : QObject(collection), m_item(item)
{
    Q_ASSERT(item);
    m_objectPath.setPath(collection->objectPath().path() + "/" + item->id());

    new orgFreedesktopSecret::ItemAdaptor(this);
    QDBusConnection::sessionBus().registerObject(m_objectPath.path(), this);
}

const QDBusObjectPath &Item::objectPath() const
{
   return m_objectPath;
}

bool Item::locked() const
{
   return m_item->isLocked();
}

void Item::setAttributes(const QMap<QString, QString> &attributes)
{
   BackendReturn<void> rc = m_item->setAttributes(attributes);
   if (rc.isError()) {
      // TODO: set D-Bus error
   }
}

QMap<QString, QString> Item::attributes() const
{
   BackendReturn<QMap<QString, QString> > rc = m_item->attributes();
   if (rc.isError()) {
      // TODO: set D-Bus error
   }
   return rc.value();
}

void Item::setLabel(const QString &label)
{
   BackendReturn<void> rc = m_item->setLabel(label);
   if (rc.isError()) {
      // TODO: set D-Bus error
   }
}

QString Item::label() const
{
   BackendReturn<QString> rc = m_item->label();
   if (rc.isError()) {
      // TODO: set D-Bus error
   }
   return rc.value();
}

qulonglong Item::created() const
{
   return m_item->created().toTime_t();
}

qulonglong Item::modified() const
{
   return m_item->modified().toTime_t();
}

QDBusObjectPath Item::deleteItem()
{
   // TODO: implement using PendingCall
   return QDBusObjectPath("/");
}

Secret Item::getSecret(const QDBusObjectPath &session)
{
   Q_UNUSED(session);
   // TODO: implement
   return Secret();
}

void Item::setSecret(const Secret &secret)
{
   Q_UNUSED(secret);
   // TODO: implement
}

BackendItem *Item::backendItem() const
{
   return m_item;
}

#include "item.moc"
