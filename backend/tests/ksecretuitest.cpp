/*
 * Copyright 2010, Dario Freddi <dario.freddi@collabora.co.uk>
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

#include "ksecretuitest.h"
#include <backendmaster.h>
#include <ksecret/ksecretcollectionmanager.h>
#include <backendcollection.h>
#include <backenditem.h>
#include <ui/dialoguimanager.h>
#include <peer.h>

#include <kstandarddirs.h>
#include <qtest_kde.h>

Q_DECLARE_METATYPE(BackendCollection*)
Q_DECLARE_METATYPE(BackendItem*)

void KSecretUiTest::initTestCase()
{
    qRegisterMetaType<BackendCollection*>();
    qRegisterMetaType<BackendItem*>();
    QCA::init();
    BackendMaster *master = BackendMaster::instance();
    master->setUiManager(new DialogUiManager);
    // use special test-directory for the .ksecret files
    m_manager = new KSecretCollectionManager("share/apps/ksecretsservice-test", master);
    // remove all files in the resource directory so no previous
    // collections are present when performing the test!
    QDir dir = QDir(KGlobal::dirs()->saveLocation("ksecret"));
    QStringList entries = dir.entryList(QStringList("*.ksecret"), QDir::Files);
    Q_FOREACH(const QString &file, entries) {
        QVERIFY(dir.remove(file));
    }
    master->addManager(m_manager);
    m_collection = 0;
}

void KSecretUiTest::testCreateCollectionAsync()
{
    CollectionCreateInfo createCollectionInfo("test", Peer( QCoreApplication::applicationPid() ));
    CreateCollectionJob *createColl = m_manager->createCreateCollectionJob(createCollectionInfo);
    QSignalSpy managerSpy(m_manager, SIGNAL(collectionCreated(BackendCollection*)));
    QSignalSpy masterSpy(BackendMaster::instance(), SIGNAL(collectionCreated(BackendCollection*)));
    QTestEventLoop loop;
    QVERIFY(loop.connect(createColl, SIGNAL(result(KJob*)), SLOT(exitLoop())));
    createColl->start();
    while(!createColl->isFinished()) {
        loop.enterLoop(120);
    }

    QVERIFY(createColl->isFinished());
    QVERIFY(!createColl->isDismissed());
    QCOMPARE(createColl->error(), BackendNoError);
    QVERIFY(createColl->collection() != 0);

    // Verify signals
    QCOMPARE(managerSpy.count(), 1);
    QCOMPARE(managerSpy.takeFirst().at(0).value<BackendCollection*>(), createColl->collection());
    QCOMPARE(masterSpy.count(), 1);
    QCOMPARE(masterSpy.takeFirst().at(0).value<BackendCollection*>(), createColl->collection());

    // Check the collection is present and alive
    BackendMaster *master = BackendMaster::instance();
    QCOMPARE(master->collections().size(), 1);
    QCOMPARE(master->collections().first(), createColl->collection());
    QCOMPARE(master->collections().first()->label().value(), QLatin1String("test"));

    // TODO: check collection attributes (eg. timestamps)

    // check that the collection has been written to disk
    QStringList entries = QDir(KGlobal::dirs()->saveLocation("ksecret")).entryList(
        QStringList("*.ksecret"), QDir::Files);
    QCOMPARE(entries.count(), 1);
    QCOMPARE(entries.at(0), QString(createColl->collection()->id() + QLatin1String(".ksecret")));

    // remember the collection
    m_collection = createColl->collection();
}

void KSecretUiTest::testLockCollectionAsync()
{
    QVERIFY( m_collection != 0);
    LockCollectionJob* lockJob = m_collection->createLockJob();
    QTestEventLoop loop;
    QVERIFY(loop.connect(lockJob, SIGNAL(result(KJob*)), SLOT(exitLoop())));
    lockJob->start();
    if (!lockJob->isFinished()) {
        loop.enterLoop(200);
    }
    QVERIFY(lockJob->isFinished());
    QVERIFY( m_collection->isLocked() ); // the test above must create the collection in locked state
}

void KSecretUiTest::testUnlockCollectionAsync()
{
    QVERIFY( m_collection->isLocked() ); // the test above must create the collection in locked state
    CollectionUnlockInfo unlockInfo( Peer( QCoreApplication::applicationPid() ));
    UnlockCollectionJob *unlockColl = m_collection->createUnlockJob(unlockInfo);
    BackendMaster *master = BackendMaster::instance();
    QSignalSpy masterSpy(master, SIGNAL(collectionChanged(BackendCollection*)));
    QSignalSpy managerSpy(m_manager, SIGNAL(collectionChanged(BackendCollection*)));
    QSignalSpy collSpy(m_collection, SIGNAL(collectionChanged(BackendCollection*)));
    QTestEventLoop loop;
    QVERIFY(loop.connect(unlockColl, SIGNAL(result(KJob*)), SLOT(exitLoop())));
    unlockColl->start();
    if (!unlockColl->isFinished()) {
        loop.enterLoop(200);
    }

    QVERIFY(unlockColl->isFinished());
    QCOMPARE(unlockColl->error(), BackendNoError);
    QVERIFY(!unlockColl->isDismissed());
    QVERIFY(unlockColl->result());
    QVERIFY(!m_collection->isLocked());

    // Verify signals
    QCOMPARE(managerSpy.count(), 1);
    QCOMPARE(managerSpy.takeFirst().at(0).value<BackendCollection*>(), m_collection);
    QCOMPARE(masterSpy.count(), 1);
    QCOMPARE(masterSpy.takeFirst().at(0).value<BackendCollection*>(), m_collection);
    QCOMPARE(collSpy.count(), 1);
    QCOMPARE(collSpy.takeFirst().at(0).value<BackendCollection*>(), m_collection);
}

void KSecretUiTest::cleanupTestCase()
{
    // TODO: delete stuff so this can also be used for valgrind leak-checking
}

QTEST_KDEMAIN(KSecretUiTest, GUI)
#include "ksecretuitest.moc"
