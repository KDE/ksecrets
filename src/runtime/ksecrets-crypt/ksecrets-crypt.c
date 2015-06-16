/* This file is part of the KDE project
 *
 * Copyright (C) 2015 Valentin Rusu <kde@rusu.info>
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

#include "ksecrets-crypt.h"

#include <syslog.h>

/*  http://stackoverflow.com/questions/14548748/encrypting-a-file-from-a-password-using-libgcrypt */

#define KSS_LOG_INFO (LOG_AUTH | LOG_INFO)

bool kss_set_credentials(const char* password)
{
  syslog(KSS_LOG_INFO, "kss_set_credentials");
  if (0 == password) {
    syslog(KSS_LOG_INFO, "NULL password given. ksecrets will not be available.");
    return false;
  }

  return true;
}

void kss_delete_credentials()
{
  syslog(KSS_LOG_INFO, "kss_delete_credentials");
}

bool kss_can_change_password() {
  /* nothing to do for the moment */
  syslog(KSS_LOG_INFO, "kss_can_change_password");
  return true;
}

bool kss_change_password(const char* password)
{
  syslog(LOG_INFO, "kss_change_password");
  return true;
}
