/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
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

#include "session.h"
#include "dbus/sessionadaptor.h"
#include "service.h"

#include <secrettool.h>

#include <QtDBus/QDBusConnection>

Session::Session(Service *parent)
 : QObject(parent),
   m_objectPath(parent->objectPath().path() + "/session/" + createId())
{
   // register on the bus
   new orgFreedesktopSecret::SessionAdaptor(this);
   QDBusConnection::sessionBus().registerObject(m_objectPath.path(), this);
}

Session *Session::create(const QString &algorithm, const QVariant &input,
                         const QVariant &output, Service *parent)
{
   Q_UNUSED(input);
   Q_UNUSED(output);
   // FIXME
   if (algorithm == "plain") {
      Session *session = new Session(parent);
      return session;
   }
   return 0;
}

Secret Session::encrypt(const QCA::SecureArray &value)
{
   // TODO: implement
   Secret s;
   s.setSession(m_objectPath);
   s.setValue(value.toByteArray());
   return s;
}

QCA::SecureArray Session::decrypt(const Secret &secret)
{
   // TODO: implement
   // make sure this is really meant for us
   Q_ASSERT(secret.objectPath() == m_objectPath);
   return secret.value();
}

void Session::close()
{
   deleteLater();
}

#include "session.moc"
