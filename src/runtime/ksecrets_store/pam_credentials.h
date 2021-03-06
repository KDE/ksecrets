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
#ifndef KSECRETS_CREDENTIALS_H
#define KSECRETS_CREDENTIALS_H

#include <ksecrets_store_export.h>

#ifdef __cplusplus
extern "C" {
#endif

int kss_set_credentials(const char* user_name, const char* password, const char* path);

int kss_delete_credentials();

int kss_can_change_password();

int kss_change_password(const char* newPassword);

#ifdef __cplusplus
}
#endif

#endif
