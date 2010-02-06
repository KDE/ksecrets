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

#include "prompt.h"
#include "service.h"
#include "dbus/promptadaptor.h"

#include <backend/backendcollection.h>
#include <backend/backenditem.h>
#include <secrettool.h>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusObjectPath>

PromptBase::PromptBase(Service *service, QObject *parent)
 : QObject(parent), m_prompted(false), m_serviceObjectPath(service->objectPath())
{
   Q_ASSERT(service);
   m_objectPath.setPath(service->objectPath().path() + "/prompts/" + createId());

   new orgFreedesktopSecret::PromptAdaptor(this);
   QDBusConnection::sessionBus().registerObject(m_objectPath.path(), this);
}

const QDBusObjectPath &PromptBase::objectPath() const
{
   return m_objectPath;
}

void PromptBase::prompt(const QString &windowId)
{
   Q_ASSERT(!m_prompted);
   Q_ASSERT(!m_pendingCalls.isEmpty());

   m_prompted = true;
   
   // TODO: convert windowId to a WId
   Q_FOREACH(AsyncCall *call, m_pendingCalls) {
      call->enqueue(windowId);
   }
}

void PromptBase::dismiss()
{
   Q_ASSERT(!m_prompted);
   Q_ASSERT(!m_pendingCalls.isEmpty());

   m_prompted = true;

   Q_FOREACH(AsyncCall *call, m_pendingCalls) {
      call->dismiss();
   }
}

const QDBusObjectPath &PromptBase::serviceObjectPath() const
{
   return m_serviceObjectPath;
}

void PromptBase::addCall(AsyncCall *call)
{
   Q_ASSERT(call);
   Q_ASSERT(!m_prompted);

   m_pendingCalls.append(call);
   connect(call, SIGNAL(completed(AsyncCall*, bool)), SLOT(slotCompleted(AsyncCall*, bool)));
}

void PromptBase::slotCompleted(AsyncCall *call, bool dismissed)
{
   Q_ASSERT(m_pendingCalls.contains[call]);
   m_pendingCalls.removeAll(call);
   if (m_pendingCalls.size() > 0) {
      return; // still waiting for more calls to complete
   }

   // TODO: handle errors
   if (dismissed) {
      emit completed(true, QVariant());
   } else {
      emit completed(false, getResult());
   }

   deleteLater();
}

PromptServiceCreateCollection::PromptServiceCreateCollection(const QString &label, bool locked,
                                                             Service *service, QObject *parent)
 : PromptBase(service, parent)
{
   // TODO: find out which manager to use.
   m_call = new AsyncCreateCollection(label, locked, 0);
   addCall(m_call);
}

QVariant PromptServiceCreateCollection::getResult() const
{
   QDBusObjectPath result("/");
   if (m_call->result()) {
      result.setPath(serviceObjectPath().path() + "/collection/" + m_call->result()->id());
   }
   
   return qVariantFromValue(result);
}

PromptServiceUnlock::PromptServiceUnlock(const QMap<BackendBase*, QDBusObjectPath> &objects,
                                         Service *service, QObject *parent)
 : PromptBase(service, parent), m_objects(objects)
{
   AsyncSimpleBooleanCall *call;

   // TODO: get rid of the cast
   Q_FOREACH(BackendBase *object, objects.keys()) {
      call = new AsyncSimpleBooleanCall(AsyncCall::AsyncUnlock, object);
      addCall(call);
      m_calls.append(call);
   }
}

QVariant PromptServiceUnlock::getResult() const
{
   QList<QDBusObjectPath> paths;

   Q_FOREACH(AsyncSimpleBooleanCall *call, m_calls) {
      if (call->result()) {
         if (m_objects.contains(call->destination())) {
            paths.append(m_objects[call->destination()]);
         }
      }
   }

   return qVariantFromValue(paths);
}

PromptServiceLock::PromptServiceLock(
   const QMap<BackendBase*, QDBusObjectPath> &objects,
   Service *service, QObject *parent)
 : PromptBase(service, parent), m_objects(objects)
{
   AsyncSimpleBooleanCall *call;

   Q_FOREACH(BackendBase *object, objects.keys()) {
      call = new AsyncSimpleBooleanCall(AsyncCall::AsyncLock, object);
      addCall(call);
      m_calls.append(call);
   }
}

QVariant PromptServiceLock::getResult() const
{
   QList<QDBusObjectPath> paths;

   Q_FOREACH(AsyncSimpleBooleanCall *call, m_calls) {
      if (call->result()) {
         if (m_objects.contains(call->destination())) {
            paths.append(m_objects[call->destination()]);
         }
      }
   }

   return qVariantFromValue(paths);
}

PromptCollectionDelete::PromptCollectionDelete(BackendCollection *collection, Service *service,
                                               QObject *parent)
 : PromptBase(service, parent)
{
   m_call = new AsyncSimpleBooleanCall(AsyncCall::AsyncDeleteCollection, collection);
   addCall(m_call);
}

QVariant PromptCollectionDelete::getResult() const
{
   return m_call->result();
}

PromptCollectionCreateItem::PromptCollectionCreateItem(BackendCollection *collection,
                                                       const QString &label,
                                                       const QMap<QString, QString> &attributes,
                                                       bool locked, const QCA::SecureArray &secret,
                                                       bool replace, Service *service,
                                                       QObject *parent)
 : PromptBase(service, parent)
{
   m_call = new AsyncCreateItem(label, attributes, secret, locked, replace, collection);
   addCall(m_call);
}

QVariant PromptCollectionCreateItem::getResult() const
{
   if (!m_call->result()) {
      // no item created
      return qVariantFromValue(QDBusObjectPath("/"));
   } else {
      return qVariantFromValue(QDBusObjectPath(serviceObjectPath().path() + "/collection/" +
         m_call->collection()->id() + "/" + m_call->result()->id()));
   }
}

PromptItemDelete::PromptItemDelete(BackendItem *item, Service *service, QObject *parent)
 : PromptBase(service, parent)
{
   m_call = new AsyncSimpleBooleanCall(AsyncCall::AsyncDeleteItem, item);
   addCall(m_call);
}

QVariant PromptItemDelete::getResult() const
{
   return m_call->result();
}

#include "prompt.moc"
