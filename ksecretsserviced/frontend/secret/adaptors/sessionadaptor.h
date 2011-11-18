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

#ifndef ORG_FREEDESKTOP_SECRET_SESSIONADAPTOR_H
#define ORG_FREEDESKTOP_SECRET_SESSIONADAPTOR_H

#include <QtDBus/QDBusAbstractAdaptor>

class Session;

namespace orgFreedesktopSecret
{

/**
 * D-Bus adaptor class for Session objects.
 */
class SessionAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Secret.Session")

public:
    /**
     * Constructor.
     *
     * @param session session object to attach the adaptor to
     */
    SessionAdaptor(Session *session);

public Q_SLOTS:
    void Close();

private:
    Session *m_session;
};

}

#endif
