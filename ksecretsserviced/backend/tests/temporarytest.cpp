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

#include "temporarytest.h"
#include <backendmaster.h>
#include <temporary/temporarycollectionmanager.h>
#include <backendcollection.h>
#include <backenditem.h>
#include <ui/nouimanager.h>
#include <jobinfostructs.h>

Q_DECLARE_METATYPE(BackendCollection*)
Q_DECLARE_METATYPE(BackendItem*)

TemporaryTest::TemporaryTest(QObject* parent): QObject(parent)
{

}

TemporaryTest::~TemporaryTest()
{

}

void TemporaryTest::initTestCase()
{
    qRegisterMetaType<BackendCollection*>();
    qRegisterMetaType<BackendItem*>();
    QCA::init();
    m_master = BackendMaster::instance();
    m_master->setUiManager(new NoUiManager);
    m_manager = new TemporaryCollectionManager(m_master);
    m_master->addManager(m_manager);
}

void TemporaryTest::testCreateCollectionSync()
{
/*    CollectionCreateInfo createCollectionInfo("test", Peer());
    CreateCollectionJob *createColl = m_manager->createCreateCollectionJob(createCollectionInfo);
    QSignalSpy managerSpy(m_manager, SIGNAL(collectionCreated(BackendCollection*)));
    QSignalSpy masterSpy(m_master, SIGNAL(collectionCreated(BackendCollection*)));

    QVERIFY(createColl->isImmediate());

    createColl->exec();

    QVERIFY(createColl->isFinished());
    QCOMPARE(createColl->error(), BackendNoError);
    QVERIFY(!createColl->isDismissed());
    QVERIFY(createColl->collection() != 0);

    // Verify signals
    QCOMPARE(managerSpy.count(), 1);
    QCOMPARE(managerSpy.takeFirst().at(0).value<BackendCollection*>()->label().value(), QString("test"));
    QCOMPARE(masterSpy.count(), 1);
    QCOMPARE(masterSpy.takeFirst().at(0).value<BackendCollection*>(), createColl->collection());

    // Check the collection is present and alive
    QCOMPARE(m_master->collections().size(), 1);
    QCOMPARE(m_master->collections().first(), createColl->collection());
    QCOMPARE(m_master->collections().first()->label().value(), QLatin1String("test"));*/
}

void TemporaryTest::testCreateItemSync()
{
/*    BackendCollection *collection = m_master->collections().first();
    QMap<QString, QString> attr;
    attr["mainattr"] = "haha";
    QCA::SecureArray array(4, 'c');
    ItemCreateInfo createInfo("testitem", attr, array, "testcontent", false, false, Peer());
    CreateItemJob *createItem = collection->createCreateItemJob(createInfo);
    QSignalSpy collectionSpy(collection, SIGNAL(itemCreated(BackendItem*)));

    QVERIFY(createItem->isImmediate());

    createItem->exec();

    QVERIFY(createItem->isFinished());
    QCOMPARE(createItem->error(), BackendNoError);
    QVERIFY(!createItem->isDismissed());
    QVERIFY(createItem->item() != 0);

    // Verify signals
    QCOMPARE(collectionSpy.count(), 1);
    QCOMPARE(collectionSpy.takeFirst().at(0).value<BackendItem*>(), createItem->item());

    // Check the item is present and alive
    QCOMPARE(collection->items().value().size(), 1);
    QCOMPARE(collection->items().value().first(), createItem->item());
    QCOMPARE(collection->items().value().first()->secret().value().toByteArray(), QByteArray("cccc"));
    QCOMPARE(collection->items().value().first()->contentType().value(), QString("testcontent"));*/
}

void TemporaryTest::testReplaceItemSync()
{
/*    BackendCollection *collection = m_master->collections().first();
    QMap<QString, QString> attr;
    attr["mainattr"] = "haha";
    QCA::SecureArray array(QByteArray("arealsecrete243"));
    ItemCreateInfo createInfo("testitem2", attr, array, "testcontent", true, false, Peer());
    CreateItemJob *createItem = collection->createCreateItemJob(createInfo);

    QSignalSpy collectionSpy(collection, SIGNAL(itemChanged(BackendItem*)));

    QVERIFY(createItem->isImmediate());

    createItem->exec();

    QVERIFY(createItem->isFinished());
    QCOMPARE(createItem->error(), BackendNoError);
    QVERIFY(!createItem->isDismissed());
    QVERIFY(createItem->item() != 0);

    // Verify signals
    QCOMPARE(collectionSpy.count(), 1);
    QCOMPARE(collectionSpy.takeFirst().at(0).value<BackendItem*>(), createItem->item());

    // Check the item is present and alive
    QCOMPARE(collection->items().value().size(), 1);
    QCOMPARE(collection->items().value().first(), createItem->item());
    QCOMPARE(collection->items().value().first()->secret().value().toByteArray(), QByteArray("arealsecrete243"));
    QCOMPARE(collection->items().value().first()->contentType().value(), QString("testcontent"));*/
}

void TemporaryTest::testDoNotReplaceItemSync()
{
/*    BackendCollection *collection = m_master->collections().first();
    QMap<QString, QString> attr;
    attr["mainattr"] = "haha";
    QCA::SecureArray array(QByteArray("anothersecret"));
    ItemCreateInfo createInfo("testitem3", attr, array, "testcontent", false, false, Peer());
    CreateItemJob *createItem = collection->createCreateItemJob(createInfo);

    QSignalSpy collectionSpy(collection, SIGNAL(itemChanged(BackendItem*)));

    QVERIFY(createItem->isImmediate());

    createItem->exec();

    QVERIFY(createItem->isFinished());
    QCOMPARE(createItem->error(), BackendErrorAlreadyExists);
    QVERIFY(!createItem->isDismissed());
    QVERIFY(createItem->item() == 0);

    // Verify signals
    QCOMPARE(collectionSpy.count(), 0);

    // Check the item is present and alive
    QCOMPARE(collection->items().value().size(), 1);
    QCOMPARE(collection->items().value().first()->secret().value().toByteArray(), QByteArray("arealsecrete243"));
    QCOMPARE(collection->items().value().first()->contentType().value(), QString("testcontent"));*/
}

void TemporaryTest::testDeleteItemSync()
{
/*    BackendCollection *collection = m_master->collections().first();
    BackendItem *item = collection->items().value().first();
    ItemDeleteInfo deleteInfo = ItemDeleteInfo(Peer());
    DeleteItemJob *deleteItem = item->createDeleteJob(deleteInfo);
    QSignalSpy collectionSpy(collection, SIGNAL(itemDeleted(BackendItem*)));

    QVERIFY(deleteItem->isImmediate());

    deleteItem->exec();

    QVERIFY(deleteItem->isFinished());
    QCOMPARE(deleteItem->error(), BackendNoError);
    QVERIFY(deleteItem->result());
    QVERIFY(!deleteItem->isDismissed());

    // Verify signals
    QCOMPARE(collectionSpy.count(), 1);
    QCOMPARE(collectionSpy.takeFirst().at(0).value<BackendItem*>()->label().value(), QString("testitem2"));

    // Check the item is present and alive
    QVERIFY(collection->items().value().isEmpty());*/
}

void TemporaryTest::testDeleteCollectionSync()
{
/*    BackendCollection *collection = m_master->collections().first();
    CollectionDeleteInfo deleteInfo = CollectionDeleteInfo(Peer());
    DeleteCollectionJob *deleteCollection = collection->createDeleteJob(deleteInfo);
    QSignalSpy managerSpy(m_manager, SIGNAL(collectionDeleted(BackendCollection*)));
    QSignalSpy masterSpy(m_master, SIGNAL(collectionDeleted(BackendCollection*)));

    QVERIFY(deleteCollection->isImmediate());

    deleteCollection->exec();

    QVERIFY(deleteCollection->isFinished());
    QCOMPARE(deleteCollection->error(), BackendNoError);
    QVERIFY(deleteCollection->result());
    QVERIFY(!deleteCollection->isDismissed());

    // Verify signals
    QCOMPARE(managerSpy.count(), 1);
    QCOMPARE(managerSpy.takeFirst().at(0).value<BackendCollection*>(), collection);
    QCOMPARE(masterSpy.count(), 1);
    QCOMPARE(masterSpy.takeFirst().at(0).value<BackendCollection*>(), collection);

    // Check the collection is dead
    QVERIFY(m_master->collections().isEmpty());*/
}

void TemporaryTest::testCreateCollectionAsync()
{
    CollectionCreateInfo createCollectionInfo("test", Peer());
    CreateCollectionJob *createColl = m_manager->createCreateCollectionJob(createCollectionInfo);
    QSignalSpy managerSpy(m_manager, SIGNAL(collectionCreated(BackendCollection*)));
    QSignalSpy masterSpy(m_master, SIGNAL(collectionCreated(BackendCollection*)));
    QEventLoop loop;
    QVERIFY(loop.connect(createColl, SIGNAL(result(KJob*)), SLOT(quit())));
    createColl->start();
    if(!createColl->isFinished()) {
        loop.exec();
    }

    QVERIFY(createColl->isFinished());
    QCOMPARE(createColl->error(), BackendNoError);
    QVERIFY(!createColl->isDismissed());
    QVERIFY(createColl->collection() != 0);

    // Verify signals
    QCOMPARE(managerSpy.count(), 1);
    QCOMPARE(managerSpy.takeFirst().at(0).value<BackendCollection*>()->label().value(), QString("test"));
    QCOMPARE(masterSpy.count(), 1);
    QCOMPARE(masterSpy.takeFirst().at(0).value<BackendCollection*>(), createColl->collection());

    // Check the collection is present and alive
    QCOMPARE(m_master->collections().size(), 1);
    QCOMPARE(m_master->collections().first(), createColl->collection());
    QCOMPARE(m_master->collections().first()->label().value(), QLatin1String("test"));
}

void TemporaryTest::testCreateItemAsync()
{
    BackendCollection *collection = m_master->collections().first();
    QMap<QString, QString> attr;
    attr["mainattr"] = "haha";
    QCA::SecureArray array(4, 'c');
    ItemCreateInfo createInfo("testitem", attr, array, "", false, false, Peer());
    CreateItemJob *createItem = collection->createCreateItemJob(createInfo);
    QSignalSpy collectionSpy(collection, SIGNAL(itemCreated(BackendItem*)));
    QEventLoop loop;
    QVERIFY(loop.connect(createItem, SIGNAL(result(KJob*)), SLOT(quit())));
    createItem->start();
    if(!createItem->isFinished()) {
        loop.exec();
    }

    QVERIFY(createItem->isFinished());
    QCOMPARE(createItem->error(), BackendNoError);
    QVERIFY(!createItem->isDismissed());
    QVERIFY(createItem->item() != 0);

    // Verify signals
    QCOMPARE(collectionSpy.count(), 1);
    QCOMPARE(collectionSpy.takeFirst().at(0).value<BackendItem*>(), createItem->item());

    // Check the item is present and alive
    QCOMPARE(collection->items().value().size(), 1);
    QCOMPARE(collection->items().value().first(), createItem->item());
    QCOMPARE(collection->items().value().first()->secret().value().toByteArray(), QByteArray("cccc"));
}

void TemporaryTest::testReplaceItemAsync()
{
    BackendCollection *collection = m_master->collections().first();
    QMap<QString, QString> attr;
    attr["mainattr"] = "haha";
    QCA::SecureArray array(QByteArray("arealsecrete243"));
    ItemCreateInfo createInfo("testitem2", attr, array, "", true, false, Peer());
    CreateItemJob *createItem = collection->createCreateItemJob(createInfo);

    QSignalSpy collectionSpy(collection, SIGNAL(itemChanged(BackendItem*)));
    QEventLoop loop;
    QVERIFY(loop.connect(createItem, SIGNAL(result(KJob*)), SLOT(quit())));
    createItem->start();
    if(!createItem->isFinished()) {
        loop.exec();
    }

    QVERIFY(createItem->isFinished());
    QCOMPARE(createItem->error(), BackendNoError);
    QVERIFY(!createItem->isDismissed());
    QVERIFY(createItem->item() != 0);

    // Verify signals
    QCOMPARE(collectionSpy.count(), 1);
    QCOMPARE(collectionSpy.takeFirst().at(0).value<BackendItem*>(), createItem->item());

    // Check the item is present and alive
    QCOMPARE(collection->items().value().size(), 1);
    QCOMPARE(collection->items().value().first(), createItem->item());
    QCOMPARE(collection->items().value().first()->secret().value().toByteArray(), QByteArray("arealsecrete243"));
}

void TemporaryTest::testDoNotReplaceItemAsync()
{
    BackendCollection *collection = m_master->collections().first();
    QMap<QString, QString> attr;
    attr["mainattr"] = "haha";
    QCA::SecureArray array(QByteArray("anothersecret"));
    ItemCreateInfo createInfo("testitem3", attr, array, "", false, false, Peer());
    CreateItemJob *createItem = collection->createCreateItemJob(createInfo);

    QSignalSpy collectionSpy(collection, SIGNAL(itemChanged(BackendItem*)));
    QEventLoop loop;
    QVERIFY(loop.connect(createItem, SIGNAL(result(KJob*)), SLOT(quit())));
    createItem->start();
    if(!createItem->isFinished()) {
        loop.exec();
    }

    QVERIFY(createItem->isFinished());
    QCOMPARE(createItem->error(), BackendErrorAlreadyExists);
    QVERIFY(!createItem->isDismissed());
    QVERIFY(createItem->item() == 0);

    // Verify signals
    QCOMPARE(collectionSpy.count(), 0);

    // Check the item is present and alive
    QCOMPARE(collection->items().value().size(), 1);
    QCOMPARE(collection->items().value().first()->secret().value().toByteArray(), QByteArray("arealsecrete243"));
}

void TemporaryTest::testDeleteItemAsync()
{
    BackendCollection *collection = m_master->collections().first();
    BackendItem *item = collection->items().value().first();
    ItemDeleteInfo deleteInfo = ItemDeleteInfo(Peer());
    DeleteItemJob *deleteItem = item->createDeleteJob(deleteInfo);
    QSignalSpy collectionSpy(collection, SIGNAL(itemDeleted(BackendItem*)));
    QEventLoop loop;
    QVERIFY(loop.connect(deleteItem, SIGNAL(result(KJob*)), SLOT(quit())));
    deleteItem->start();
    if(!deleteItem->isFinished()) {
        loop.exec();
    }

    QVERIFY(deleteItem->isFinished());
    QCOMPARE(deleteItem->error(), BackendNoError);
    QVERIFY(deleteItem->result());
    QVERIFY(!deleteItem->isDismissed());

    // Verify signals
    QCOMPARE(collectionSpy.count(), 1);
    QCOMPARE(collectionSpy.takeFirst().at(0).value<BackendItem*>()->label().value(), QString("testitem2"));

    // Check if item is gone
    QVERIFY(collection->items().value().isEmpty());
}

void TemporaryTest::testDeleteCollectionAsync()
{
    BackendCollection *collection = m_master->collections().first();
    CollectionDeleteInfo deleteInfo = CollectionDeleteInfo(Peer());
    DeleteCollectionJob *deleteCollection = collection->createDeleteJob(deleteInfo);
    QSignalSpy managerSpy(m_manager, SIGNAL(collectionDeleted(BackendCollection*)));
    QSignalSpy masterSpy(m_master, SIGNAL(collectionDeleted(BackendCollection*)));
    QEventLoop loop;
    QVERIFY(loop.connect(deleteCollection, SIGNAL(result(KJob*)), SLOT(quit())));
    deleteCollection->start();
    if(!deleteCollection->isFinished()) {
        loop.exec();
    }

    QVERIFY(deleteCollection->isFinished());
    QCOMPARE(deleteCollection->error(), BackendNoError);
    QVERIFY(deleteCollection->result());
    QVERIFY(!deleteCollection->isDismissed());

    // Verify signals
    QCOMPARE(managerSpy.count(), 1);
    QCOMPARE(managerSpy.takeFirst().at(0).value<BackendCollection*>(), collection);
    QCOMPARE(masterSpy.count(), 1);
    QCOMPARE(masterSpy.takeFirst().at(0).value<BackendCollection*>(), collection);

    // Check the collection is dead
    QVERIFY(m_master->collections().isEmpty());
}

void TemporaryTest::cleanupTestCase()
{
    // TODO: delete stuff so this can also be used for valgrind leak-checking.
}


QTEST_MAIN(TemporaryTest)
#include "temporarytest.moc"
