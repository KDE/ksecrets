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

#include "ksecretsvalue.h"
#include "ksecretsvalue_p.h"

using namespace KSecrets;

const char* Secret::CONTENT_TYPE_PASSWORD = "password";

Secret::Secret() :
    d( new SecretPrivate() )
{
}

Secret::Secret(SecretPrivate* sp) :
    d( sp )
{
}

Secret::Secret( const Secret& that ) :
    d( that.d )
{
}

Secret::~Secret() {
}

bool Secret::operator ! () const
{
    return !d;
}

Secret& Secret::operator=(const Secret& that)
{
    d = that.d;
    return *this;
}

QVariant Secret::value() const {
    return d->value;
}

QString Secret::contentType() const
{
    return d->contentType;
}

void Secret::setValue( const QVariant &value, const QString &contentType ) {
    d->value = value;
    d->contentType = contentType;
}

void Secret::setValue( const QVariant &val ) {
    d->value = val;
    d->contentType = QStringLiteral( "QVariant" );
}

bool Secret::operator==(const Secret& that) const
{
    return *d == *that.d;
}

SecretPrivate::SecretPrivate()
{
}

SecretPrivate::SecretPrivate( const SecretPrivate &that ) :
    QSharedData( that )
{
    contentType = that.contentType;
    value = that.value;
}

SecretPrivate::SecretPrivate( const DBusSecretStruct &that )
{
    value = that.m_value;
    contentType = that.m_contentType;
}

SecretPrivate::~SecretPrivate()
{
}

bool SecretPrivate::operator == ( const SecretPrivate &that )  const
{
    bool result = contentType == that.contentType;
    result &= value == that.value;
    return result;
}


#include "ksecretsvalue.moc"
