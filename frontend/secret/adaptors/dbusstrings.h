/*
 * Copyright 2010, Valentin Rusu <kde@rusu.info>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ORG_FREEDESKTOP_SECRET_DBUSSTRINGS_H
#define ORG_FREEDESKTOP_SECRET_DBUSSTRINGS_H

/** \file dbusstrings.h
 *  \brief KSecretService DBus frequently used strings by clients
 *
 *  This file aims to reduce typo problems when using strings to
 *  construct DBus bindings in client programs.
 *
 *  DBUS_SERVICE_XXX macros define service instances
 *  DBUS_INTERFACE_SECRET_XXX macros define interface instances
 *  DBUS_PATH_SECRET_XXX macros define path to objects
 */

/**
 * KSecretService service instance
 */
#define DBUS_SERVICE_SECRET "org.freedesktop.Secret"

#define DBUS_INTERFACE_SECRET_SERVICE   "org.freedesktop.Secret.Service"
#define DBUS_INTERFACE_SECRET_SESSION   "org.freedesktop.Secret.Session"

#define DBUS_PATH_SECRETS           "/org/freedesktop/secrets"
#define DBUS_PATH_SECRET_SESSION    "/org/freedesktop/secrets/session"

#endif // ORG_FREEDESKTOP_SECRET_DBUSSTRINGS_H
