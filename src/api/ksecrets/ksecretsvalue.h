/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
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

#ifndef KSECRETSVALUE_H
#define KSECRETSVALUE_H

#include <ksecrets_export.h>
#include <QVariant>
#include <QSharedDataPointer>


namespace KSecrets {

class SecretPrivate;
class Secret;
typedef QSharedPointer<Secret> SecretPtr;

/**
 * This class holds the secret information your application wants to store
 * inside a secrets collection.
 */
class KSECRETS_EXPORT Secret {
    explicit Secret( SecretPrivate* sp );
public:
    Secret();
    Secret( const Secret & that );
    virtual ~Secret();

    /**
     * Used to test if this Secret instance is correctly initialized.
     * @return true if this Secret instance is not correctly initialized
     */
    bool operator ! () const;

    Secret& operator = (const Secret& that);

    /**
     * Returns a QVariant containing the secret value
     */
    QVariant value() const;

    /**
     * Returns the content type of the value stored inside this secret
     */
    QString contentType() const;

    /**
     * Convenience string used to qualify the content type when storing password secrets
     */
    static const char* CONTENT_TYPE_PASSWORD;

    /**
     * Set this secrets value alongside with it's content type
     */
    void setValue( const QVariant &value, const QString &contentType );

    /**
     * Convenience method used when no content type is available
     * @see setValue( const QVariant &value, const QString &contentType )
     */
    void setValue( const QVariant &value );

    /**
     * Tests equality between this secret's value and content type and that's secret same properties
     */
    bool operator == ( const Secret& that ) const;

private:
    QSharedDataPointer< SecretPrivate > d;
};

};

#endif // KSECRETSVALUE_H
