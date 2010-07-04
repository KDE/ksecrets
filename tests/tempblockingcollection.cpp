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

#include "tempblockingcollection.h"
#include "tempblockingitem.h"

#include <secrettool.h>

TempBlockingCollection::TempBlockingCollection(const QString &id, BackendCollectionManager *parent)
 : TemporaryCollection(id, parent)
{
}

TempBlockingCollection::~TempBlockingCollection()
{
}

BackendReturn<QList<BackendItem*> > TempBlockingCollection::items() const
{
   return m_items;
}

BackendReturn<QList<BackendItem*> > TempBlockingCollection::searchItems(
   const QMap<QString, QString> &attributes) const
{
   TempBlockingItem *titem;
   QList<BackendItem*> foundItems;
   Q_FOREACH(BackendItem *item, m_items) {
      titem = qobject_cast<TempBlockingItem*>(item);
      if (titem && titem->matches(attributes)) {
         foundItems.append(item);
      }
   }
   return foundItems;
}

bool TempBlockingCollection::isCallImmediate(AsyncCall::AsyncType type) const
{
   Q_UNUSED(type);
   return false;
}

BackendReturn<BackendItem*> TempBlockingCollection::createItem(const QString &label,
                                                               const QMap<QString, QString> &attributes,
                                                               const QCA::SecureArray &secret,
                                                               bool replace, bool locked)
{
   Q_UNUSED(locked);

   TempBlockingItem *item = 0;
   bool replacing = false;

   // check for duplicates
   BackendReturn<QList<BackendItem*> > foundItems = searchItems(attributes);
   if (!foundItems.isError() && foundItems.value().size() > 0) {
      QList<BackendItem*> oldlist = foundItems.value();
      Q_FOREACH(BackendItem* olditem, oldlist) {
         if (olditem->attributes().value() == attributes) {
            if (replace) {
               // replacing an existing item
               item = qobject_cast<TempBlockingItem*>(olditem);
               replacing = true;
            } else {
               // item existing but should not be replaced
               return BackendReturn<BackendItem*>(0, ErrorAlreadyExists);
            }
            break;
         }
      }
   }

   if (!item) {
      item = new TempBlockingItem(createId(), this);
   }
   item->setLabel(label);
   item->setAttributes(attributes);
   item->setSecret(secret);

   if (replacing) {
      emit itemChanged(item);
   } else {
      m_items.append(item);
      // new item, signals need to be wired
      connect(item, SIGNAL(itemDeleted(BackendItem*)), SLOT(slotItemDeleted(BackendItem*)));
      connect(item, SIGNAL(itemChanged(BackendItem*)), SIGNAL(itemChanged(BackendItem*)));
      emit itemCreated(item);
   }

   return item;
}

void TempBlockingCollection::slotItemDeleted(BackendItem *item)
{
   m_items.removeAll(item);
   emit itemDeleted(item);
}

#include "tempblockingcollection.moc"
