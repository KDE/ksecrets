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

#ifndef KSECRETSSECRET_P_H
#define KSECRETSSECRET_P_H

#include "ksecretsvalue.h"

#include <QtDBus/QDBusObjectPath>
#include <QSharedData>

namespace KSecrets {

class SecretPrivate : public QSharedData {
public:
    SecretPrivate();
    SecretPrivate( const SecretPrivate& that );
    ~SecretPrivate();


    bool operator == ( const SecretPrivate &that ) const;

    QString contentType;
    QVariant value;
};

} // namespace

#endif // KSECRETSSECRET_P_H
