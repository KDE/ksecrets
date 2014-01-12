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

#include "protocoltest.h"
#include "../syncprotocol.h"

#include <QtTest/QTest>
#include <qprocess.h>
#include <QtNetwork/QTcpSocket>
#include <QtTest/QSignalSpy>
#include <QTimer>
#include <kdebug.h>
#include <qtest_kde.h>

QTEST_KDEMAIN( ProtocolTest, NoGUI )


ProtocolTest::ProtocolTest() 
{

}

void ProtocolTest::createLogEntry(const QString& logEntry)
{
    kDebug() << logEntry;
}

void ProtocolTest::initTestCase()
{
}

void ProtocolTest::testProtocol()
{
    SyncProtocolClient *client = new SyncProtocolClient( this );
    SyncProtocolServer *server = new SyncProtocolServer( this );
    
    forever {
        QString request = client->nextRequest();
        QString response;
        QVERIFY(server->handleRequest( request, response ) );
        QVERIFY(client->handleReply(response));
        
        if ( client->isDone() )
            break;
    }
}

void ProtocolTest::cleanupTestCase()
{
}

#include "protocoltest.moc"
