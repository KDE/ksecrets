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

#ifndef KSECRETSITEM_H
#define KSECRETSITEM_H

#include "ksecretsvalue.h"
#include <QSharedData>
#include <QFuture>
#include <QDateTime>
#include <ksecrets_export.h>
#include <qwindowdefs.h>

namespace KSecrets {

typedef QMap<QString, QString> AttributesMap;

class SecretItemPrivate;
class SecretItem;
typedef QSharedPointer<SecretItem> SecretItemPtr;

/**
 * KSecrets aims to let application store sensitive pieces of information as
 *SecretItem(s).
 * The central part of a SecretItem is the secret it holds. The secret is
 *actually a structure named @ref SecretStruct["(SecretStruct)"]
 * SecretItems can be qualified using attributes. These attributes are used
 *internally by KSecrets to uniquely identify them inside the collection.
 * The attributes list always contain at least one item, named "Label". It's
 *content is up to the client application.
 * The "Label" attribute can also be read by calling the @ref attribute method
 *and set by @ref setLabel method.
 *
 * Please note that all the jobs returned by this class autodelete themselbes
 *when done. If you application
 * need to access the returned items, then it should copy them away before
 *returning from the job's done
 * signal handling method.
 */
class KSECRETS_EXPORT SecretItem : public QSharedData {
  SecretItem(SecretItemPrivate*);

  public:
  SecretItem();
  SecretItem(const SecretItem&);
  virtual ~SecretItem();

  /**
   */
  QFuture<bool> deleteItem(QWidget* parent = NULL);

  /**
   * Read the data held by the SecretItem
   */
  QFuture<SecretPtr> getSecret() const;

  /**
   * Modify the item's stored data
   */
  QFuture<bool> setSecret(const Secret& secret);

  /**
   * @note returned ReadItemPropertyJob::value is a QMap< QString, QString>
   */
  QFuture<AttributesMap> attributes() const;

  /**
   * @param attributes a map containing the new attributes; it must contain at
   * least one attribute, under the name "Label"
   */
  QFuture<bool> setAttributes(const AttributesMap& attributes);

  /**
   * @note returned ReadItemPropertyJob::value is a bool
   */
  QFuture<bool> isLocked() const;

  /**
   * @note returned ReadItemPropertyJob::value is a QString
   */
  QFuture<QString> label() const;

  /**
   * @note returned ReadItemPropertyJob::value is a time_t
   */
  QFuture<QDateTime> createdTime() const;

  /**
   * @note returned ReadItemPropertyJob::value is a time_t
   */
  QFuture<QDateTime> modifiedTime() const;

  /**
   * Sets the item's label
   */
  QFuture<bool> setLabel(const QString& label);

  private:
  friend class SecretItemPrivate;

  QSharedDataPointer<SecretItemPrivate> d;
};

}

Q_DECLARE_METATYPE(KSecrets::SecretItem)

#endif // KSECRETSITEM_H
