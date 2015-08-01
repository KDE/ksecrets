/*
    This file is part of the KDE Libraries

    Copyright (C) 2015 Valentin Rusu (valir@kde.org)

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
#ifndef KSECRETSCOLLECTIONDBUS_P
#define KSECRETSCOLLECTIONDBUS_P

#include "../ksecretscollection_p.h"

namespace KSecrets {

class CollectionPrivateDbus : public CollectionPrivate {
  CollectionPrivateDbus();
  virtual ~CollectionPrivateDbus();
  void setDBusPath(const QDBusObjectPath& collPath);
  bool isValid() const;

  OrgFreedesktopSecretCollectionInterface* collectionInterface();
  static void notifyCollectionDeleted(const QDBusObjectPath&);
  static void notifyCollectionChanged(const QDBusObjectPath&);

  static QMap<QDBusObjectPath, CollectionPrivate*> collectionMap;
  QDBusObjectPath dbusPath;

private:
  OrgFreedesktopSecretCollectionInterface* collectionIf;
};

}

#endif
