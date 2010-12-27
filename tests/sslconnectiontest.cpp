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

#include "sslconnectiontest.h"

#include <QtTest/QTest>
#include <qprocess.h>
#include <QtNetwork/QSslSocket>
#include <QtTest/QSignalSpy>
#include <QTimer>
#include <kdebug.h>
#include <qtest_kde.h>

QTEST_KDEMAIN( SslConnectionTest, NoGUI )

SslConnectionTest::SslConnectionTest()
{

}

void SslConnectionTest::initTestCase()
{
/*    QString program = "../ksecretsync";
    QStringList arguments;
    arguments << "--nofork";
    _daemonProcess = new QProcess(this);
    _daemonProcess->start( program, arguments );
    QVERIFY( _daemonProcess->waitForStarted() );*/
}

void SslConnectionTest::testSslConnection()
{
    QSslSocket* socket = new QSslSocket(this);
    QSignalSpy encryptedSpy( socket, SIGNAL( encrypted() ) );
    QVERIFY( encryptedSpy.isValid() );
    
    socket->connectToHostEncrypted( "zamox.rusu.info", 8383 );
    if ( !socket->waitForEncrypted( 6000 ) ) {
        kDebug() << socket->errorString();
        QVERIFY( false );
    }
    
    socket->close();
    delete socket;
}

void SslConnectionTest::cleanupTestCase()
{
/*    if ( _daemonProcess )
        _daemonProcess->close();*/
}
