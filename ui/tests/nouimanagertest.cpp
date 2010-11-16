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

#include "nouimanagertest.h"

#include <QtCrypto/QtCrypto>
#include <QtTest/QTestEventLoop>
#include <qtest_kde.h>

#include "../nouimanager.h"

void NoUiManagerTest::initTestCase()
{
    QCA::init();
}

void NoUiManagerTest::testAskPassword()
{
    NoUiManager manager;

    // create a job and start it
    AbstractAskPasswordJob *job = manager.createAskPasswordJob("TESTCOLLECTION", false);
    QVERIFY(!job->isImmediate());

    QTestEventLoop loop;
    connect(job, SIGNAL(result(QueuedJob*)), &loop, SLOT(exitLoop()));
    job->enqueue();
    loop.enterLoop(5);
    QVERIFY(!loop.timeout());

    // verify job result
    QVERIFY(job->isFinished());
    QVERIFY(!job->cancelled());
    QCOMPARE(job->password().toByteArray(), QByteArray("default"));

    // verify job deletion
    connect(job, SIGNAL(destroyed(QObject*)), &loop, SLOT(exitLoop()));
    loop.enterLoop(5);
    QVERIFY(!loop.timeout());
}

void NoUiManagerTest::testNewPassword()
{
    NoUiManager manager;

    // create a job and start it
    AbstractNewPasswordJob *job = manager.createNewPasswordJob("TESTCOLLECTION");
    QVERIFY(!job->isImmediate());

    QTestEventLoop loop;
    connect(job, SIGNAL(result(QueuedJob*)), &loop, SLOT(exitLoop()));
    job->enqueue();
    loop.enterLoop(5);
    QVERIFY(!loop.timeout());

    // verify job result
    QVERIFY(job->isFinished());
    QVERIFY(!job->cancelled());
    QCOMPARE(job->password().toByteArray(), QByteArray("default"));

    // verify job deletion
    connect(job, SIGNAL(destroyed(QObject*)), &loop, SLOT(exitLoop()));
    loop.enterLoop(5);
    QVERIFY(!loop.timeout());
}

void NoUiManagerTest::testJobOrder()
{
    NoUiManager manager;

    // create 3 jobs
    AbstractNewPasswordJob *job1 = manager.createNewPasswordJob("1");
    AbstractNewPasswordJob *job2 = manager.createNewPasswordJob("2");
    AbstractNewPasswordJob *job3 = manager.createNewPasswordJob("3");

    // enqueue the job so they should end up in order 1 -> 2 -> 3
    job2->enqueue();
    job3->enqueue();
    job1->enqueue(true);

    QTestEventLoop loop;
    connect(job1, SIGNAL(result(QueuedJob*)), &loop, SLOT(exitLoop()));
    connect(job2, SIGNAL(result(QueuedJob*)), &loop, SLOT(exitLoop()));
    connect(job3, SIGNAL(result(QueuedJob*)), &loop, SLOT(exitLoop()));

    // check if jobs are executed in the right order
    // NOTE: once a job is done it will be deleted, so successive tests
    //       for isFinished() might fail on them.
    loop.enterLoop(5);
    QVERIFY(job1->isFinished() && !job2->isFinished() && !job3->isFinished());

    loop.enterLoop(5);
    QVERIFY(job2->isFinished() && !job3->isFinished());

    loop.enterLoop(5);
    QVERIFY(job3->isFinished());
}

void NoUiManagerTest::testAskPasswordCancelled()
{
    NoUiManager manager;
    manager.setCancelAll(true);

    // create a job and start it
    AbstractAskPasswordJob *job = manager.createAskPasswordJob("TESTCOLLECTION", false);
    QVERIFY(!job->isImmediate());

    QTestEventLoop loop;
    connect(job, SIGNAL(result(QueuedJob*)), &loop, SLOT(exitLoop()));
    job->enqueue();
    loop.enterLoop(5);
    QVERIFY(!loop.timeout());

    // verify job result
    QVERIFY(job->isFinished());
    QVERIFY(job->cancelled());
    QCOMPARE(job->password().toByteArray(), QByteArray(""));

    // verify job deletion
    connect(job, SIGNAL(destroyed(QObject*)), &loop, SLOT(exitLoop()));
    loop.enterLoop(5);
    QVERIFY(!loop.timeout());
}

void NoUiManagerTest::testNewPasswordCancelled()
{
    NoUiManager manager;
    manager.setCancelAll(true);

    // create a job and start it
    AbstractNewPasswordJob *job = manager.createNewPasswordJob("TESTCOLLECTION");
    QVERIFY(!job->isImmediate());

    QTestEventLoop loop;
    connect(job, SIGNAL(result(QueuedJob*)), &loop, SLOT(exitLoop()));
    job->enqueue();
    loop.enterLoop(5);
    QVERIFY(!loop.timeout());

    // verify job result
    QVERIFY(job->isFinished());
    QVERIFY(job->cancelled());
    QCOMPARE(job->password().toByteArray(), QByteArray(""));

    // verify job deletion
    connect(job, SIGNAL(destroyed(QObject*)), &loop, SLOT(exitLoop()));
    loop.enterLoop(5);
    QVERIFY(!loop.timeout());
}

void NoUiManagerTest::testJobOrderCancelled()
{
    NoUiManager manager;
    manager.setCancelAll(true);

    // create 3 jobs
    AbstractNewPasswordJob *job1 = manager.createNewPasswordJob("1");
    AbstractNewPasswordJob *job2 = manager.createNewPasswordJob("2");
    AbstractNewPasswordJob *job3 = manager.createNewPasswordJob("3");

    // enqueue the job so they should end up in order 1 -> 2 -> 3
    job2->enqueue();
    job3->enqueue();
    job1->enqueue(true);

    QTestEventLoop loop;
    connect(job1, SIGNAL(result(QueuedJob*)), &loop, SLOT(exitLoop()));
    connect(job2, SIGNAL(result(QueuedJob*)), &loop, SLOT(exitLoop()));
    connect(job3, SIGNAL(result(QueuedJob*)), &loop, SLOT(exitLoop()));

    // check if jobs are executed in the right order
    // NOTE: once a job is done it will be deleted, so successive tests
    //       for isFinished() might fail on them.
    loop.enterLoop(5);
    QVERIFY(job1->isFinished() && !job2->isFinished() && !job3->isFinished());

    loop.enterLoop(5);
    QVERIFY(job2->isFinished() && !job3->isFinished());

    loop.enterLoop(5);
    QVERIFY(job3->isFinished());
}

void NoUiManagerTest::cleanupTestCase()
{
}

QTEST_KDEMAIN(NoUiManagerTest, GUI)
#include "nouimanagertest.moc"
