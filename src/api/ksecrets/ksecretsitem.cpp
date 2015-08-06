/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <valir@kde.org>
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

#include "ksecretsitem.h"
#include "ksecretsitem_p.h"

#include <QDateTime>
#include <QtConcurrent>

using namespace KSecrets;

SecretItem::SecretItem()
    : d(new SecretItemPrivate())
{
}

SecretItem::SecretItem(const SecretItem& that)
    : QSharedData(that)
    , d(that.d)
{
}

SecretItem::SecretItem(SecretItemPrivate* sip)
    : d(sip)
{
}

SecretItem::~SecretItem() {}

QFuture<bool> SecretItem::deleteItem(QWidget* parent)
{
  return QtConcurrent::run(d.data(), &SecretItemPrivate::deleteItem, parent);
}

QFuture<SecretPtr> SecretItem::getSecret() const
{
  return QtConcurrent::run(d.data(), &SecretItemPrivate::getSecret);
}

QFuture<bool> SecretItem::setSecret(const Secret& secret)
{
  return QtConcurrent::run(d.data(), &SecretItemPrivate::setSecret, secret);
}

QFuture<AttributesMap> SecretItem::attributes() const
{
  return QtConcurrent::run(d.data(), &SecretItemPrivate::attributes);
}

QFuture<bool> SecretItem::setAttributes(
    const QMap<QString, QString>& attributes)
{
  return QtConcurrent::run(
      d.data(), &SecretItemPrivate::setAttributes, attributes);
}

QFuture<bool> SecretItem::isLocked() const
{
  return QtConcurrent::run(d.data(), &SecretItemPrivate::isLocked);
}

QFuture<QString> SecretItem::label() const
{
  return QtConcurrent::run(d.data(), &SecretItemPrivate::label);
}

QFuture<QDateTime> SecretItem::createdTime() const
{
  return QtConcurrent::run(d.data(), &SecretItemPrivate::createdTime);
}

QFuture<QDateTime> SecretItem::modifiedTime() const
{
  return QtConcurrent::run(d.data(), &SecretItemPrivate::modifiedTime);
}

QFuture<bool> SecretItem::setLabel(const QString& label)
{
  return QtConcurrent::run(d.data(), &SecretItemPrivate::setLabel, label);
}

SecretItemPrivate::SecretItemPrivate() {}

SecretItemPrivate::~SecretItemPrivate() {}

SecretItemPrivate::SecretItemPrivate(const SecretItemPrivate& that)
    : QSharedData(that)
{
}

bool SecretItemPrivate::isValid() const
{
  // TODO figure out if something must be checked, if not, just return true
  return true;
}

bool SecretItemPrivate::deleteItem(QWidget*) {
  // TODO
  return true;
}

SecretPtr SecretItemPrivate::getSecret() const {
  // TODO
  return SecretPtr();
}

bool SecretItemPrivate::setSecret(const Secret& secret) {
  // TODO
  return true;
}

AttributesMap SecretItemPrivate::attributes() const {
  // TODO
  return AttributesMap();
}

bool SecretItemPrivate::setAttributes(const AttributesMap &)
{
  // TODO
  return true;
}

bool SecretItemPrivate::isLocked() const {
  return true; // no lock semantics for this version, however it acts like it's always locked
}

QString SecretItemPrivate::label() const {
  // TODO
  return QLatin1Literal("");
}

bool SecretItemPrivate::setLabel(const QString&) {
  // TODO
  return true;
}

QDateTime SecretItemPrivate::createdTime() const
{
  // TODO
  return QDateTime();
}

QDateTime SecretItemPrivate::modifiedTime() const
{
  // TODO
  return QDateTime();
}

#include "ksecretsitem.moc"
