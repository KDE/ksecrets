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

#ifndef SERVICETEST_H
#define SERVICETEST_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtDBus/QDBusObjectPath>

class BackendMaster;
class Service;
class QDBusInterface;
class QDBusAbstractInterface;

/**
 * Unit-test for the fd.o Secret daemon
 */
class ServiceTest : public QObject
{
   Q_OBJECT

private:
   BackendMaster *m_master;
   Service *m_service;

private Q_SLOTS:
   // create services and collections needed for testing
   void initTestCase();

   // check if Service is registered with D-Bus
   void dbusService();

   // open various sessions, make sure they are available on the bus, then close them.
   void session();

   // create and remove collections
   void nonBlockingCollection();

   // cleanup
   void cleanupTestCase();
};

/**
 * Listens to D-Bus signals which only have an object-path
 * as only argument.
 */
class ObjectPathSignalSpy : public QObject, public QList<QDBusObjectPath>
{
   Q_OBJECT

public:
   // constructor. similar to QSignalSpy's constructor.
   ObjectPathSignalSpy(QDBusAbstractInterface *iface, const char *signal);
   
   // return true if the connection to the signal was successful
   bool isValid() const;
   
   // wait a specified number of ms for the signal to be received.
   void waitForSignal(int time);

Q_SIGNALS:
   // stop waiting for the signal.
   void doneWaiting();
   
private Q_SLOTS:
   // Single slot which receives all of the various signals.
   void slotReceived(const QDBusObjectPath &objectPath);
   
private:
   bool m_valid;
};

#endif
