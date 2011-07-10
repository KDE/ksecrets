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

#ifndef SECRETSTRUCT_H
#define SECRETSTRUCT_H

#include <QDBusObjectPath>
#include <QSharedData>
#include <QDBusArgument>

/**
 * This is the basic Secret structure exchanged via the dbus API
 * See the spec for more details
 */
struct SecretStruct {
    QDBusObjectPath m_session;
    QByteArray      m_parameters;
    QByteArray      m_value;
    QString         m_contentType;
};

Q_DECLARE_METATYPE( SecretStruct )

inline QDBusArgument &operator<<(QDBusArgument &argument, const SecretStruct &secret)
{
    argument.beginStructure();
    argument << secret.m_session << secret.m_parameters << secret.m_value << secret.m_contentType;
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, SecretStruct &secret)
{
    argument.beginStructure();
    argument >> secret.m_session >> secret.m_parameters >> secret.m_value >> secret.m_contentType;
    argument.endStructure();
    return argument;
}

#endif // SECRETSTRUCT_H
