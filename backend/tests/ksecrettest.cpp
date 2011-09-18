/*
 * Copyright 2010, Dario Freddi <dario.freddi@collabora.co.uk>
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

#include "ksecrettest.h"
#include <backendmaster.h>
#include <ksecret/ksecretcollectionmanager.h>
#include <backendcollection.h>
#include <backenditem.h>
#include <ui/nouimanager.h>
#include <peer.h>

#include <kstandarddirs.h>

Q_DECLARE_METATYPE(BackendCollection*)
Q_DECLARE_METATYPE(BackendItem*)

void KSecretTest::initTestCase()
{
    qRegisterMetaType<BackendCollection*>();
    qRegisterMetaType<BackendItem*>();
    QCA::init();
    BackendMaster *master = BackendMaster::instance();
    master->setUiManager(new NoUiManager);
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
}

void KSecretTest::testCreateCollectionAsync()
{
    CollectionCreateInfo createCollectionInfo("test", Peer( QCoreApplication::applicationPid() ));
    CreateCollectionJob *createColl = m_manager->createCreateCollectionJob(createCollectionInfo);
    QSignalSpy managerSpy(m_manager, SIGNAL(collectionCreated(BackendCollection*)));
    QSignalSpy masterSpy(BackendMaster::instance(), SIGNAL(collectionCreated(BackendCollection*)));
    QTestEventLoop loop;
    QVERIFY(loop.connect(createColl, SIGNAL(result(KJob*)), SLOT(exitLoop())));
    createColl->start();
    if(!createColl->isFinished()) {
        loop.enterLoop(5);
    }

    QVERIFY(createColl->isFinished());
    QCOMPARE(createColl->error(), BackendNoError);
    QVERIFY(!createColl->isDismissed());
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
    m_collCreated = m_collection->created();
    m_collModified = m_collection->modified();
}

void KSecretTest::testLockCollectionAsync()
{
    LockCollectionJob *lockColl = m_collection->createLockJob();
    BackendMaster *master = BackendMaster::instance();
    QSignalSpy masterSpy(master, SIGNAL(collectionChanged(BackendCollection*)));
    QSignalSpy managerSpy(m_manager, SIGNAL(collectionChanged(BackendCollection*)));
    QSignalSpy collSpy(m_collection, SIGNAL(collectionChanged(BackendCollection*)));
    QTestEventLoop loop;
    QVERIFY(loop.connect(lockColl, SIGNAL(result(KJob*)), SLOT(exitLoop())));
    lockColl->start();
    if(!lockColl->isFinished()) {
        loop.enterLoop(5);
    }

    QVERIFY(lockColl->isFinished());
    QCOMPARE(lockColl->error(), BackendNoError);
    QVERIFY(!lockColl->isDismissed());
    QVERIFY(lockColl->result());
    QVERIFY(m_collection->isLocked());

    // Verify signals
    QCOMPARE(managerSpy.count(), 1);
    QCOMPARE(managerSpy.takeFirst().at(0).value<BackendCollection*>(), m_collection);
    QCOMPARE(masterSpy.count(), 1);
    QCOMPARE(masterSpy.takeFirst().at(0).value<BackendCollection*>(), m_collection);
    QCOMPARE(collSpy.count(), 1);
    QCOMPARE(collSpy.takeFirst().at(0).value<BackendCollection*>(), m_collection);
}

void KSecretTest::testUnlockCollectionAsync()
{
    CollectionUnlockInfo unlockInfo = CollectionUnlockInfo(Peer( QCoreApplication::applicationPid() ));
    UnlockCollectionJob *unlockColl = m_collection->createUnlockJob(unlockInfo);
    BackendMaster *master = BackendMaster::instance();
    QSignalSpy masterSpy(master, SIGNAL(collectionChanged(BackendCollection*)));
    QSignalSpy managerSpy(m_manager, SIGNAL(collectionChanged(BackendCollection*)));
    QSignalSpy collSpy(m_collection, SIGNAL(collectionChanged(BackendCollection*)));
    QTestEventLoop loop;
    QVERIFY(loop.connect(unlockColl, SIGNAL(result(KJob*)), SLOT(exitLoop())));
    unlockColl->start();
    if (!unlockColl->isFinished()) {
        loop.enterLoop(5);
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

void KSecretTest::testCreateItemAsync()
{
    QMap<QString, QString> attr;
    attr["mainattr"] = "haha";
    QCA::SecureArray array(4, 'c');
    ItemCreateInfo createInfo("testitem", attr, array, "", false, false, Peer());
    CreateItemJob *createItem = m_collection->createCreateItemJob(createInfo);
    QSignalSpy collectionSpy(m_collection, SIGNAL(itemCreated(BackendItem*)));
    QTestEventLoop loop;
    QVERIFY(loop.connect(createItem, SIGNAL(result(KJob*)), SLOT(exitLoop())));
    createItem->start();
    if (!createItem->isFinished()) {
        loop.enterLoop(5);
    }
    
    QVERIFY(createItem->isFinished());
    QCOMPARE(createItem->error(), BackendNoError);
    QVERIFY(!createItem->isDismissed());
    QVERIFY(createItem->item() != 0);
    
    // Verify signals
    QCOMPARE(collectionSpy.count(), 1);
    QCOMPARE(collectionSpy.takeFirst().at(0).value<BackendItem*>(), createItem->item());
    
    // Check the item is present and alive
    QCOMPARE(m_collection->items().value().size(), 1);
    QCOMPARE(m_collection->items().value().first(), createItem->item());
    QCOMPARE(m_collection->items().value().first()->secret().value().toByteArray(), QByteArray("cccc"));
    
    // remember the item
    m_item = createItem->item();
}

void KSecretTest::testCheckItem()
{
    // check attributes
    QVERIFY(m_item && !m_item->isLocked());
    QVERIFY(!m_item->id().isEmpty());
    BackendReturn<QString> ret1 = m_item->label();
    QVERIFY(!ret1.isError());
    QCOMPARE(ret1.value(), QString("testitem"));
    BackendReturn<QCA::SecureArray> ret2 = m_item->secret();
    QVERIFY(!ret2.isError());
    QCOMPARE(ret2.value().toByteArray(), QByteArray("cccc"));
    BackendReturn<QMap<QString, QString> > ret3 = m_item->attributes();
    QVERIFY(!ret3.isError());
    QCOMPARE(ret3.value().size(), 1);
    QCOMPARE(ret3.value().value("mainattr"), QLatin1String("haha"));
    
    // check timestamps
    QDateTime cur = QDateTime::currentDateTimeUtc();
    QVERIFY(m_item->created() >= m_collCreated);
    QVERIFY(cur >= m_item->created());
    QVERIFY(m_item->modified() >= m_collCreated);
    QVERIFY(cur >= m_item->modified());
    
    // remember item creation and modification times
    m_itemCreated = m_item->created();
    m_itemModified = m_item->modified();
}

void KSecretTest::testSearchItem()
{
    // search for the item we just created and find it.
    QMap<QString, QString> attr1;
    attr1["mainattr"] = "haha";
    BackendReturn<QList<BackendItem*> > ret1 = m_collection->searchItems(attr1);
    QVERIFY(!ret1.isError());
    QCOMPARE(ret1.value().size(), 1);
    QCOMPARE(ret1.value().first(), m_item);
    
    // search for another item and DON'T find it.
    QMap<QString, QString> attr2;
    attr2["mainattr"] = "hoho";
    BackendReturn<QList<BackendItem*> > ret2 = m_collection->searchItems(attr2);
    QVERIFY(!ret2.isError());
    QVERIFY(ret2.value().isEmpty());
}

void KSecretTest::testChangeItem()
{
    QVERIFY(m_item != 0);
    QDateTime start = QDateTime::currentDateTimeUtc();
    
    // change the item we created earlier
    BackendReturn<void> ret1 = m_item->setLabel("itemtest");
    QVERIFY(!ret1.isError());
    BackendReturn<void> ret2 = m_item->setSecret(QCA::SecureArray(4, 'b'));
    QVERIFY(!ret2.isError());
    QMap<QString, QString> attr1;
    attr1["mainattr"] = "huhu";
    attr1["secoattr"] = "hihi";
    BackendReturn<void> ret3 = m_item->setAttributes(attr1);
    
    // check the values match with what we set
    BackendReturn<QString> ret4 = m_item->label();
    QVERIFY(!ret4.isError());
    QCOMPARE(ret4.value(), QString("itemtest"));
    BackendReturn<QCA::SecureArray> ret5 = m_item->secret();
    QVERIFY(!ret5.isError());
    QCOMPARE(ret5.value().toByteArray(), QByteArray("bbbb"));
    BackendReturn<QMap<QString, QString> > ret6 = m_item->attributes();
    QVERIFY(!ret6.isError());
    QCOMPARE(ret6.value().size(), 2);
    QCOMPARE(ret6.value().value("mainattr"), QString("huhu"));
    QCOMPARE(ret6.value().value("secoattr"), QString("hihi"));
    
    QDateTime end = QDateTime::currentDateTimeUtc();
    
    // make sure the item's creation time didn't change and the modification
    // time changed.
    QCOMPARE(m_item->created(), m_itemCreated);
    QVERIFY(m_item->modified() >= start);
    QVERIFY(m_item->modified() <= end);
    QCOMPARE(m_collection->created(), m_collCreated);
    QVERIFY(m_collection->modified() >= start);
    QVERIFY(m_collection->modified() <= end);
    
    // change the item back so the tests still work
    QVERIFY(!m_item->setLabel("testitem").isError());
    QMap<QString, QString> attr2;
    attr2["mainattr"] = "haha";
    QVERIFY(!m_item->setAttributes(attr2).isError());
}

void KSecretTest::testReplaceItemAsync()
{
    QMap<QString, QString> attr;
    attr["mainattr"] = "haha";
    QCA::SecureArray array(QByteArray("arealsecrete243"));
    ItemCreateInfo createInfo("testitem2", attr, array, "", true, false, Peer());
    CreateItemJob *createItem = m_collection->createCreateItemJob(createInfo);
    QSignalSpy collectionSpy(m_collection, SIGNAL(itemChanged(BackendItem*)));
    QTestEventLoop loop;
    QVERIFY(loop.connect(createItem, SIGNAL(result(KJob*)), SLOT(exitLoop())));
    createItem->start();
    if (!createItem->isFinished()) {
        loop.enterLoop(5);
    }
    
    QVERIFY(createItem->isFinished());
    QCOMPARE(createItem->error(), BackendNoError);
    QVERIFY(!createItem->isDismissed());
    QVERIFY(createItem->item() != 0);
    
    // Verify signals
    QCOMPARE(collectionSpy.count(), 1);
    QCOMPARE(collectionSpy.takeFirst().at(0).value<BackendItem*>(), createItem->item());
    
    // Check the item is present and alive
    QCOMPARE(m_collection->items().value().size(), 1);
    QCOMPARE(m_collection->items().value().first(), createItem->item());
    QCOMPARE(m_collection->items().value().first()->secret().value().toByteArray(), QByteArray("arealsecrete243"));
}

void KSecretTest::testDoNotReplaceItemAsync()
{
    QMap<QString, QString> attr;
    attr["mainattr"] = "haha";
    QCA::SecureArray array(QByteArray("anothersecret"));
    ItemCreateInfo createInfo("testitem3", attr, array, "", false, false, Peer());
    CreateItemJob *createItem = m_collection->createCreateItemJob(createInfo);
    QSignalSpy collectionSpy(m_collection, SIGNAL(itemChanged(BackendItem*)));
    QTestEventLoop loop;
    QVERIFY(loop.connect(createItem, SIGNAL(result(KJob*)), SLOT(exitLoop())));
    createItem->start();
    if (!createItem->isFinished()) {
        loop.enterLoop(5);
    }
    
    QVERIFY(createItem->isFinished());
    QCOMPARE(createItem->error(), BackendErrorAlreadyExists);
    QVERIFY(!createItem->isDismissed());
    QVERIFY(createItem->item() == 0);
    
    // Verify signals
    QCOMPARE(collectionSpy.count(), 0);
    
    // Check the item is present and alive
    QCOMPARE(m_collection->items().value().size(), 1);
    QCOMPARE(m_collection->items().value().first()->secret().value().toByteArray(), QByteArray("arealsecrete243"));
}

void KSecretTest::testDeleteItemAsync()
{
    QVERIFY(m_collection != 0);
    QVERIFY(m_collection->items().value().count() >0);
    BackendItem *item = m_collection->items().value().first();
    ItemDeleteInfo deleteInfo = ItemDeleteInfo(Peer());
    DeleteItemJob *deleteItem = item->createDeleteJob(deleteInfo);
    QSignalSpy collectionSpy(m_collection, SIGNAL(itemDeleted(BackendItem*)));
    QTestEventLoop loop;
    QVERIFY(loop.connect(deleteItem, SIGNAL(result(KJob*)), SLOT(exitLoop())));
    deleteItem->start();
    if (!deleteItem->isFinished()) {
        loop.enterLoop(5);
    }
    
    QVERIFY(deleteItem->isFinished());
    QCOMPARE(deleteItem->error(), BackendNoError);
    QVERIFY(deleteItem->result());
    QVERIFY(!deleteItem->isDismissed());
    
    // Verify signals
    QCOMPARE(collectionSpy.count(), 1);
    QCOMPARE(collectionSpy.takeFirst().at(0).value<BackendItem*>()->label().value(), QString("testitem2"));
    
    // check if the item is gone
    QVERIFY(m_collection->items().value().isEmpty());
}

void KSecretTest::testDeleteCollectionAsync()
{
    BackendMaster *master = BackendMaster::instance();
    CollectionDeleteInfo deleteInfo = CollectionDeleteInfo(Peer());
    DeleteCollectionJob *deleteCollection = m_collection->createDeleteJob(deleteInfo);
    QSignalSpy managerSpy(m_manager, SIGNAL(collectionDeleted(BackendCollection*)));
    QSignalSpy masterSpy(master, SIGNAL(collectionDeleted(BackendCollection*)));
    QTestEventLoop loop;
    QVERIFY(loop.connect(deleteCollection, SIGNAL(result(KJob*)), SLOT(exitLoop())));
    deleteCollection->start();
    if (!deleteCollection->isFinished()) {
        loop.enterLoop(5);
    }
    
    QVERIFY(deleteCollection->isFinished());
    QCOMPARE(deleteCollection->error(), BackendNoError);
    QVERIFY(deleteCollection->result());
    QVERIFY(!deleteCollection->isDismissed());
    
    // Verify signals
    QCOMPARE(managerSpy.count(), 1);
    QCOMPARE(managerSpy.takeFirst().at(0).value<BackendCollection*>(), m_collection);
    QCOMPARE(masterSpy.count(), 1);
    QCOMPARE(masterSpy.takeFirst().at(0).value<BackendCollection*>(), m_collection);
    
    // check the collection is dead
    QVERIFY(master->collections().isEmpty());
    
    // check that the collection has been removed from disk
    QStringList entries = QDir(KGlobal::dirs()->saveLocation("ksecret")).entryList(
        QStringList("*.ksecret"), QDir::Files);
    QCOMPARE(entries.count(), 0);
}

void KSecretTest::cleanupTestCase()
{
    // TODO: delete stuff so this can also be used for valgrind leak-checking
}

QTEST_MAIN(KSecretTest)
#include "ksecrettest.moc"
