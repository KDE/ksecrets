/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KSECRETSCOLLECTION_P_H
#define KSECRETSCOLLECTION_P_H

#include "ksecretscollection.h"
#include "dbusbackend.h"

#include <QDBusObjectPath>
#include <QString>
#include <QDateTime>

class OrgFreedesktopSecretCollectionInterface;

namespace KSecrets {

typedef QMap<QString, QString> QStringMap;

class CollectionPrivate {
public:
  explicit CollectionPrivate(Collection*);
  ~CollectionPrivate();

  bool isValid();
  bool isNewlyCreated() const;
  void setDBusPath(const QDBusObjectPath& collPath);
  const WId& promptParentId() const;
  OrgFreedesktopSecretCollectionInterface* collectionInterface();
  void setStatus(Collection::Status);
  static void notifyCollectionDeleted(const QDBusObjectPath&);
  static void notifyCollectionChanged(const QDBusObjectPath&);
  bool lock();
  static QList<CollectionPtr> listCollections();
  bool deleteCollection();
  bool renameCollection(const QString&);
  QList<SecretItemPtr> searchItems(const QStringMap&);
  QList<SecretPtr> searchSecrets(const QStringMap&);
  bool createItem(const QString& label, const QStringMap& attributes,
      const Secret& secret, CreateItemOptions options);
  QList<SecretItemPtr> items();
  bool isLocked();
  QString label();
  QDateTime createdTime();
  QDateTime modifiedTime();
  bool setLabel(const QString&);
  bool writeProperty(const QString& propName, const QVariant& propVal);

public:
  Collection* collection;
  QWidget* promptParent;
  QString collectionName;
  QVariantMap collectionProperties;
  Collection::Status collectionStatus;
  QDBusObjectPath dbusPath;
  static QMap<QDBusObjectPath, CollectionPrivate*> collectionMap;

private:
  OrgFreedesktopSecretCollectionInterface* collectionIf;
};
};

#endif // KSECRETSCOLLECTION_P_H
