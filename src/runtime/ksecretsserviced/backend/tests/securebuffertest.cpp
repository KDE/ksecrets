/*
 * Copyright (C) 2010  Michael Leupold <lemma@confuego.org>
 *
 * Large portions of the design and implementation have been copied
 * from the QBuffer unit-test (tst_qbuffer.cpp):
 *   Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *   All rights reserved.
 *   Contact: Nokia Corporation (qt-info@nokia.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include "securebuffertest.h"

#include <QtTest/QtTest>

#include <securebuffer.h>

SecureBufferTest::SecureBufferTest()
{
    qRegisterMetaType<QCA::SecureArray>();
}

// Testing get/set functions
void SecureBufferTest::getSetCheck()
{
    SecureBuffer obj1;
    // const QByteArray & QBuffer::data()
    // void QBuffer::setData(const QByteArray &)
    QByteArray var1("Bogus data");
    obj1.setData(var1);
    QCOMPARE(QCA::SecureArray(var1), obj1.data());
    obj1.setData(QByteArray());
    QCOMPARE(QCA::SecureArray(), obj1.data());
    QCA::SecureArray var2("Bogus data");
    obj1.setData(var2);
    QCOMPARE(var2, obj1.data());
    obj1.buffer().fill('x');
    QVERIFY(var2 != obj1.data());
}

// some status() tests, too
void SecureBufferTest::readBlock()
{
//    QTest::ignoreMessage(QtWarningMsg, "QIODevice::read: File not open");
//    QTest::ignoreMessage(QtWarningMsg, "QIODevice::read: Read operation not permitted");

    const int arraySize = 10;
    char a[arraySize];
    SecureBuffer b;
    QCOMPARE(b.read(a, arraySize), (qint64) - 1); // not opened
    QVERIFY(b.atEnd());

    QCA::SecureArray ba;
    ba.resize(arraySize);
    b.setBuffer(&ba);
    b.open(QIODevice::WriteOnly);
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::read: WriteOnly device");
    QCOMPARE(b.read(a, arraySize), (qint64) - 1); // no read access
    b.close();

    b.open(QIODevice::ReadOnly);
    QCOMPARE(b.read(a, arraySize), (qint64) arraySize);
    QVERIFY(b.atEnd());

    // up to 3.0.x reading beyond the end was an error while ok
    // this has been made consistent with other QIODevice sub classes in 3.1
    QCOMPARE(b.read(a, 1), qint64(0));
    QVERIFY(b.atEnd());

    // read in two chunks
    b.close();
    b.open(QIODevice::ReadOnly);
    QCOMPARE(b.read(a, arraySize / 2), (qint64) arraySize / 2);
    QCOMPARE(b.read(a + arraySize / 2, arraySize - arraySize / 2),
             (qint64)(arraySize - arraySize / 2));
    QVERIFY(b.atEnd());
}

void SecureBufferTest::readBlockPastEnd()
{
    QCA::SecureArray arr(4096 + 3616, 'd');
    SecureBuffer buf(&arr);

    buf.open(QIODevice::ReadOnly);
    char dummy[4096];

    buf.read(1);

    QCOMPARE(buf.read(dummy, 4096), qint64(4096));
    QCOMPARE(buf.read(dummy, 4096), qint64(3615));
    QVERIFY(buf.atEnd());
}

void SecureBufferTest::writeBlock_data()
{
    QTest::addColumn<QString>("str");

    QTest::newRow("small_bytearray") << QString("Test");
    QTest::newRow("large_bytearray") << QString("Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                                     "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
                                     "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad"
                                     "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
                                     "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit"
                                     "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
                                     "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
}

void SecureBufferTest::writeBlock()
{
    QFETCH(QString, str);

    QCA::SecureArray ba;
    SecureBuffer buf(&ba);
    buf.open(QIODevice::ReadWrite);
    QByteArray data = str.toLatin1();
    QCOMPARE(buf.write(data.constData(), data.size()), qint64(data.size()));

    QCOMPARE(buf.data(), QCA::SecureArray(data));
}

void SecureBufferTest::seek()
{
    SecureBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QCOMPARE(buffer.size(), qint64(0));
    QCOMPARE(buffer.pos(), qint64(0));
    const qint64 pos = 10;
    QVERIFY(buffer.seek(pos));
    QCOMPARE(buffer.size(), pos);
}

void SecureBufferTest::seekTest_data()
{
    writeBlock_data();
}

#define DO_VALID_SEEK(position) {                                            \
        char c;                                                                  \
        QVERIFY(buf.seek(qint64(position)));                                      \
        QCOMPARE(buf.pos(), qint64(position));                                    \
        QVERIFY(buf.getChar(&c));                                                 \
        QCOMPARE(QChar(c), str.at(qint64(position)));                             \
    }
#define DO_INVALID_SEEK(position) {                                          \
        qint64 prev_pos = buf.pos();                                             \
        QVERIFY(!buf.seek(qint64(position)));                                     \
        QCOMPARE(buf.pos(), prev_pos); /* position should not be changed */                  \
    }

void SecureBufferTest::seekTest()
{
    QFETCH(QString, str);

    QCA::SecureArray ba;
    SecureBuffer buf(&ba);
#if 0
    QCOMPARE(buf.pos(), qint64(-1));
#endif
    buf.open(QIODevice::ReadWrite);
    QCOMPARE(buf.pos(), qint64(0));

    QByteArray data = str.toLatin1();
    QCOMPARE(buf.write(data.constData(), data.size()), qint64(data.size()));

    QTest::ignoreMessage(QtWarningMsg, "SecureBuffer::seek: Invalid pos: -1");
    DO_INVALID_SEEK(-1);

    DO_VALID_SEEK(0);
    DO_VALID_SEEK(str.size() - 1);
    QVERIFY(buf.atEnd());
    DO_VALID_SEEK(str.size() / 2);

    // Special case: valid to seek one position past the buffer.
    // Its then legal to write, but not read.
    {
        char c = 'a';
        QVERIFY(buf.seek(qint64(str.size())));
        QCOMPARE(buf.read(&c, qint64(1)), qint64(0));
        QCOMPARE(c, 'a');
        QCOMPARE(buf.write(&c, qint64(1)), qint64(1));
    }

    // Special case 2: seeking to an arbitrary position beyond the buffer auto-expands it
    // (see Task 184730)
    {
        char c;
        const int offset = 1;
        Q_ASSERT(offset > 0); // any positive integer will do
        const qint64 pos = buf.size() + offset;
        QVERIFY(buf.seek(pos));
        QCOMPARE(buf.pos(), pos);
        QVERIFY(!buf.getChar(&c));
        QVERIFY(buf.seek(pos - 1));
        QVERIFY(buf.getChar(&c));
        QCOMPARE(c, buf.data().at(pos - 1));
        QVERIFY(buf.seek(pos));
        QVERIFY(buf.putChar(c));
    }
}

void SecureBufferTest::read_rawdata()
{
    static const unsigned char mydata[] = {
        0x01, 0x00, 0x03, 0x84, 0x78, 0x9c, 0x3b, 0x76,
        0xec, 0x18, 0xc3, 0x31, 0x0a, 0xf1, 0xcc, 0x99,
        0x6d, 0x5b
    };

    QCA::SecureArray data = QByteArray::fromRawData((const char *)mydata, sizeof(mydata));
    SecureBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);
    quint8 ch;
    for(int i = 0; i < (int)sizeof(mydata); ++i) {
        QVERIFY(!buffer.atEnd());
        in >> ch;
        QVERIFY(ch == (quint8)mydata[i]);
    }
    QVERIFY(buffer.atEnd());
}

void SecureBufferTest::isSequential()
{
    SecureBuffer buf;
    QVERIFY(!buf.isSequential());
}

void SecureBufferTest::signalTest_data()
{
    QTest::addColumn<QByteArray>("sample");

    QTest::newRow("empty") << QByteArray();
    QTest::newRow("size 1") << QByteArray("1");
    QTest::newRow("size 2") << QByteArray("11");
    QTest::newRow("size 100") << QByteArray(100, '1');
}

void SecureBufferTest::signalTest()
{
    QFETCH(QByteArray, sample);

    totalBytesWritten = 0;

    SecureBuffer buf;
    buf.open(QIODevice::WriteOnly);

    buf.buffer().resize(sample.size() * 10);
    connect(&buf, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
    connect(&buf, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWrittenSlot(qint64)));

    for(int i = 0; i < 10; ++i) {
        gotReadyRead = false;
        QCOMPARE(buf.write(sample), qint64(sample.size()));
        if(sample.size() > 0) {
            QTestEventLoop::instance().enterLoop(5);
            if(QTestEventLoop::instance().timeout())
                QFAIL("Timed out when waiting for readyRead()");
            QCOMPARE(totalBytesWritten, qint64(sample.size() *(i + 1)));
            QVERIFY(gotReadyRead);
        } else {
            QCOMPARE(totalBytesWritten, qint64(0));
            QVERIFY(!gotReadyRead);
        }
    }
}

void SecureBufferTest::readyReadSlot()
{
    gotReadyRead = true;
    QTestEventLoop::instance().exitLoop();
}

void SecureBufferTest::bytesWrittenSlot(qint64 written)
{
    totalBytesWritten += written;
}

void SecureBufferTest::isClosedAfterClose()
{
    SecureBuffer buffer;
    buffer.open(SecureBuffer::ReadOnly);
    QVERIFY(buffer.isOpen());
    buffer.close();
    QVERIFY(!buffer.isOpen());
}

void SecureBufferTest::readLine_data()
{
    QTest::addColumn<QCA::SecureArray>("src");
    QTest::addColumn<int>("maxlen");
    QTest::addColumn<QByteArray>("expected");

    QTest::newRow("1") << QCA::SecureArray("line1\nline2\n") << 1024
                       << QByteArray("line1\n");
    QTest::newRow("2") << QCA::SecureArray("hi there") << 1024
                       << QByteArray("hi there");
    QTest::newRow("3") << QCA::SecureArray("l\n") << 3 << QByteArray("l\n");
    QTest::newRow("4") << QCA::SecureArray("l\n") << 2 << QByteArray("l");
}

void SecureBufferTest::readLine()
{
    QFETCH(QCA::SecureArray, src);
    QFETCH(int, maxlen);
    QFETCH(QByteArray, expected);

    SecureBuffer buf;
    buf.setBuffer(&src);
    char *result = new char[maxlen + 1];
    result[maxlen] = '\0';

    QVERIFY(buf.open(QIODevice::ReadOnly));

    qint64 bytes_read = buf.readLine(result, maxlen);

    QCOMPARE(bytes_read, qint64(expected.size()));
    QCOMPARE(QByteArray(result), expected);

    buf.close();
    delete[] result;

}

void SecureBufferTest::canReadLine_data()
{
    QTest::addColumn<QCA::SecureArray>("src");
    QTest::addColumn<bool>("expected");

    QTest::newRow("1") << QCA::SecureArray("no newline") << false;
    QTest::newRow("2") << QCA::SecureArray("two \n lines\n") << true;
    QTest::newRow("3") << QCA::SecureArray("\n") << true;
    QTest::newRow("4") << QCA::SecureArray() << false;
}

void SecureBufferTest::canReadLine()
{
    QFETCH(QCA::SecureArray, src);
    QFETCH(bool, expected);

    SecureBuffer buf;
    buf.setBuffer(&src);
    QVERIFY(!buf.canReadLine());
    QVERIFY(buf.open(QIODevice::ReadOnly));
    QCOMPARE(buf.canReadLine(), expected);
}

void SecureBufferTest::atEnd()
{
    SecureBuffer buffer;
    buffer.open(SecureBuffer::Append);
    buffer.write("heisann");
    buffer.close();

    buffer.open(SecureBuffer::ReadOnly);
    buffer.seek(buffer.size());
    char c;
    QVERIFY(!buffer.getChar(&c));
    QCOMPARE(buffer.read(&c, 1), qint64(0));
}

void SecureBufferTest::readLineBoundaries()
{
    QByteArray line = "This is a line\n";
    SecureBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    while(buffer.size() < 16384)
        buffer.write(line);

    /*
        buffer.seek(0);
        QFile out1("out1.txt");
        out1.open(QFile::WriteOnly);
        out1.write(buffer.readAll());
        out1.close();
    */
    buffer.seek(0);

    char c;
    buffer.getChar(&c);
    buffer.ungetChar(c);

    QFile out2("out2.txt");
    out2.open(QFile::WriteOnly);
    while(!buffer.atEnd())
        out2.write(buffer.readLine());

    out2.close();
    out2.remove();
}

void SecureBufferTest::writeAfterSecureArrayResize()
{
    SecureBuffer buffer;
    QVERIFY(buffer.open(QIODevice::WriteOnly));

    buffer.write(QByteArray().fill('a', 1000));
    QCOMPARE(buffer.buffer().size(), 1000);

    // resize the QByteArray behind SecureBuffer's back
    buffer.buffer().clear();
    buffer.seek(0);
    QCOMPARE(buffer.buffer().size(), 0);

    buffer.write(QByteArray().fill('b', 1000));
    QCOMPARE(buffer.buffer().size(), 1000);
}

QTEST_MAIN(SecureBufferTest)

#include "securebuffertest.moc"
