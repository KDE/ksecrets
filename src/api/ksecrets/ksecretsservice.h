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
#ifndef KSECRETSSERVICE
#define KSECRETSSERVICE

#include <QObject>
#include <QFuture>
#include <ksecrets_export.h>

#include "ksecretscollection.h"

namespace KSecrets {

class ServicePrivate;

class KSECRETS_EXPORT Service : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(Service)
  public:
  virtual ~Service();

  /**
   * Options used when findCollection method is called
   * TODO add an option to allow searching into the KWallet
   */
  enum FindCollectionOptions {
    OpenOnly = 0,        /// this will only try to open the collection without
                         /// creating it if not found
    CreateCollection = 1, /// the collection will be created if not found
  };

  /**
   * This will try to find a collection given its name. If not found, it'll
   * create it depending on the
   * options given.
   * @param collectionName collection name to be found
   * @param options @see FindCollectionOptions
   * @param collectionProperties specify collection properties and it's
   * semantics depends on the options parameter
   *      if options == CreateCollection, then the newly created collection
   * will receive these properties
   *      if options == OpenOnly, then the properties are used to match the
   * existing collection so be careful
   *                              not to specify a property not given when
   * creation a collection or you'll not be
   *                              able to find it with this method
   * @param promptParentWindowId identifies the applications window to be used
   * as a parent for prompt windows
   * @return Collection instance to be used by the client application to
   * further manipulated it's secrets
   * @note Please note that the collection returned by this method is not yet
   * connected to the secret storing
   * infrastructure. As such, a NotFound status would not be immediatley
   * known. Application should be prepared
   * to get such an error upon the execution of the first KJob returned by one
   * of the other methods of this class.
   */
  static QFuture<CollectionPtr> findCollection(const QString& collectionName,
      FindCollectionOptions options = CreateCollection,
      const QVariantMap& collectionProperties = QVariantMap(),
      QWidget* promptParent = 0);

  private:
  QSharedPointer<ServicePrivate> d;
};
}

#endif
