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

#include "socketconnectiontest.h"
#include "../syncprotocol.h"

#include <QtTest/QTest>
#include <qprocess.h>
#include <QtNetwork/QTcpSocket>
#include <QtTest/QSignalSpy>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <kdebug.h>
#include <qtest_kde.h>
#include <kprocess.h>
#include <unistd.h>

QTEST_KDEMAIN( SocketConnectionTest, NoGUI )


SocketConnectionTest::SocketConnectionTest() :
    _daemonProcess(0)
{

}

void SocketConnectionTest::createLogEntry(const QString& logEntry)
{
    kDebug() << logEntry;
}

void SocketConnectionTest::initTestCase()
{
    bool startSecretSync = true;
    // TODO: figure out why reading from /proc do not work as expected
/*    QDir procDir("/proc");
    QStringList procList = procDir.entryList();
    QStringList::const_iterator curProc;
    for ( curProc = procList.constBegin(); curProc != procList.constEnd(); ++curProc ) {
        QString procName = *curProc;
        bool isProc = false;
        int pid = procName.toInt( &isProc );
        if ( isProc && (pid >= 1000) ) {
            QFile cmdLineFile( QString("/proc/%1/cmdline").arg( pid ) );
            QVERIFY( cmdLineFile.open( QIODevice::ReadOnly | QIODevice::Unbuffered ) );
            char buffer[512];
            qint64 count = cmdLineFile.read( buffer, sizeof(buffer) );
            QVERIFY( count >0 );
            if ( QString( buffer ).startsWith( "ksecretsync" ) ) {
                kDebug() << "found ksecretsync PID " << pid;
                startSecretSync = false;
                break;
            }
        }
    }*/
    
    if ( startSecretSync ) {
        kDebug() << "launching ksecretsync";
        system( "killall 'ksecretsync'" );
        sleep(2);
        QString program = "ksecretsync";
        QStringList arguments;
        arguments << "--nofork";
        _daemonProcess = new QProcess(this);
        _daemonProcess->start( program, arguments );
        QVERIFY( _daemonProcess->waitForStarted() );
    }
}

void SocketConnectionTest::testProtocol()
{
    QTcpSocket* socket = new QTcpSocket(this);

    socket->connectToHost( "localhost", 8383 );
    
    SyncProtocolClient *protocol = new SyncProtocolClient( this );
    
    forever {
        QVERIFY( -1 != socket->write(  qPrintable( protocol->nextRequest() ) ) );
        
        QVERIFY( socket->waitForReadyRead( 30000 ) );
        
        // FIXME: adjust the size of this buffer
        char respBuffer[1024];
        size_t respLength = sizeof( respBuffer );
        qint64 lineLength = socket->readLine( respBuffer, respLength );
        QVERIFY( -1 == lineLength );
        
        QVERIFY( protocol->handleReply( QString( respBuffer ) ) );
        
        if ( protocol->isDone() )
            break;
    }
    
    socket->close();
    delete socket;
}

void SocketConnectionTest::cleanupTestCase()
{
    if ( _daemonProcess )
        _daemonProcess->close();
}

#include "socketconnectiontest.moc"
