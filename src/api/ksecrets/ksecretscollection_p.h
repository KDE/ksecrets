/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <valir@kde.org>
 * Copyright (C) 2015 Valentin Rusu <valir@kde.org>
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

#include <QString>
#include <QDateTime>

class OrgFreedesktopSecretCollectionInterface;

namespace KSecrets {

class CollectionPrivate {
  public:
  explicit CollectionPrivate(Collection*);
  ~CollectionPrivate();

  bool isValid();
  bool isNewlyCreated() const;
  const WId& promptParentId() const;
  void setStatus(Collection::Status);
  bool lock();
  static QList<CollectionPtr> listCollections();
  bool deleteCollection();
  bool renameCollection(const QString&);
  QList<SecretItemPtr> searchItems(const AttributesMap&);
  QList<SecretPtr> searchSecrets(const AttributesMap&);
  bool createItem(const QString& label, const AttributesMap& attributes,
      const Secret& secret, CreateItemOptions options);
  QList<SecretItemPtr> items() const;
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
};
}

#endif // KSECRETSCOLLECTION_P_H
