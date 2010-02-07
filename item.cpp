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
#include "session.h"
#include "secret.h"
#include "prompt.h"

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
   // bypass prompt?
   if (m_item->isCallImmediate(AsyncCall::AsyncDeleteItem)) {
      m_item->deleteItem();
      return QDBusObjectPath("/");
   } else {
      // FIXME: needs the service as well as some QObject.
      //        I'd say the whole constructor should be checked
      //        and its signature changed.
      PromptItemDelete *p = new PromptItemDelete(m_item, 0, 0);
      return p->objectPath();
   }
}

Secret Item::getSecret(const QDBusObjectPath &session)
{
   if (m_item->isLocked()) {
      // TODO: error, requires unlocking
      return Secret();
   }
   
   QObject *object = QDBusConnection::sessionBus().objectRegisteredAt(session.path());
   Session *sessionObj;
   if (object && (sessionObj = qobject_cast<Session*>(object))) {
      BackendReturn<QCA::SecureArray> br = m_item->secret();
      if (br.isError()) {
         // TODO: handle error
         return Secret();
      }
      bool ok;
      Secret secret = sessionObj->encrypt(br.value(), ok);
      if (br.isError()) {
         // TODO: error, invalid session
         return Secret();
      }
      return secret;
   } else {
      // TODO: error, requires session
      return Secret();
   }
}

void Item::setSecret(const Secret &secret)
{
   if (m_item->isLocked()) {
      // TODO: error, requires unlocking
      return;
   }
   
   QObject *object = QDBusConnection::sessionBus().objectRegisteredAt(secret.session().path());
   Session *sessionObj;
   if (object && (sessionObj = qobject_cast<Session*>(object))) {
      bool ok;
      BackendReturn<QCA::SecureArray> br = sessionObj->decrypt(secret, ok);
      if (!ok) {
         // TODO: invalid session
         return;
      }
      BackendReturn<void> rc = m_item->setSecret(br.value());
      if (rc.isError()) {
         // TODO: handle error
         return;
      }
   }
   
   // TODO: error, invalid session
   
   return;
}

BackendItem *Item::backendItem() const
{
   return m_item;
}

#include "item.moc"
