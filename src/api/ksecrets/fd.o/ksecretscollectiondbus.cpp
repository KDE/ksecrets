/*
    This file is part of the KDE Libraries

    Copyright (C) 2015 Valentin Rusu (kde@rusu.info)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ksecretscollectiondbus_p.h"

namespace KSecrets {

QMap<QDBusObjectPath, CollectionPrivate*>
    CollectionPrivateDbus::collectionMap;

CollectionPrivateDbus::CollectionPrivateDbus() :
    collectionIf(0)
{
}

CollectionPrivateDbus::~CollectionPrivateDbus()
{
  if (collectionMap.contains(dbusPath)) {
    collectionMap.remove(dbusPath);
  }
}

bool CollectionPrivateDbus::isValid()
{
  // NOTE: do not call collectionInterface() to get the interface pointer, if
  // not you'll get an infinite recursive call
  return collectionIf && collectionIf->isValid()
      && (collectionStatus == Collection::FoundExisting
             || collectionStatus == Collection::NewlyCreated);
}

void CollectionPrivateDbus::setDBusPath(const QDBusObjectPath& path)
{
  collectionIf = DBusSession::createCollectionIf(path);
  if (collectionIf->isValid()) {
    qDebug() << "SUCCESS opening collection " << path.path();
    collectionMap.insert(path, this);
    dbusPath = path;
  }
  else {
    setStatus(Collection::NotFound);
    qDebug() << "ERROR opening collection " << path.path();
  }
}

OrgFreedesktopSecretCollectionInterface*
CollectionPrivateDbus::collectionInterface()
{
  if ((collectionIf == 0) || (!collectionIf->isValid())) {
  }
  return collectionIf;
}

void CollectionPrivateDbus::notifyCollectionChanged(
    const QDBusObjectPath& path)
{
  if (collectionMap.contains(path)) {
    CollectionPrivate* cp = collectionMap[path];
    cp->collection->emitContentsChanged();
  }
  else {
    qDebug() << "Ignoring notifyCollectionChanged for " << path.path();
  }
}

void CollectionPrivateDbus::notifyCollectionDeleted(
    const QDBusObjectPath& path)
{
  if (collectionMap.contains(path)) {
    CollectionPrivate* cp = collectionMap[path];
    cp->collection->emitDeleted();
  }
  else {
    qDebug() << "Ignoring notifyCollectionDeleted for " << path.path();
  }
}
}

