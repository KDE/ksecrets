/*
    This file is part of the KDE Libraries

    Copyright (C) 2015 Valentin Rusu (valir@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ksecrets_store_test.h"

#include <ksecrets_store.h>
#include <QtTest/QtTest>
#include <QtCore/QDir>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

QTEST_GUILESS_MAIN(KSecretServiceStoreTest);

KSharedConfig::Ptr sharedConfig;
QString secretsFilePath;

KSecretServiceStoreTest::KSecretServiceStoreTest(QObject* parent)
    : QObject(parent)
{
}

void KSecretServiceStoreTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    sharedConfig = KSharedConfig::openConfig(QLatin1String("ksecretsrc"));

    secretsFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QVERIFY(QDir::home().mkpath(secretsFilePath));
    secretsFilePath += QLatin1Literal("/ksecrets-test.data");
    qDebug() << "secrets store path: " << secretsFilePath;
    // create a test file here and performe the real open test below
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData(), false);
    QVERIFY(setupfut.get());
    auto credfut = backend.setCredentials("test", "ksecrets-test:crypt", "ksecrets-test:mac");
    QVERIFY(credfut.get());
}

static const char* collName1 = "test collection1";
static const char* collName2 = "test collection2";
std::time_t createTimeMark;

void KSecretServiceStoreTest::testCreateCollection()
{
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData(), false);

    createTimeMark = std::time(nullptr);
    auto crval1 = backend.createCollection(collName1);
    QVERIFY(crval1);

    auto crval2 = backend.createCollection(collName2);
    QVERIFY(crval2);

    auto crval3 = backend.createCollection(collName1);
    QVERIFY(!crval3); // should not work as a collection named collName1 is already present

    auto coll2 = crval2.result_;
    QVERIFY(coll2.get() != nullptr);
    QVERIFY(coll2->createdTime() > createTimeMark);
    QVERIFY(coll2->modifiedTime() > createTimeMark);
    QVERIFY(coll2->createdTime() == coll2->modifiedTime());
    QVERIFY(coll2->label() == collName2);

    auto sres = coll2->dirItems();
    QVERIFY(sres.size() == 0);

    KSecretsStore::AttributesMap noAttrs;
    sres = coll2->searchItems(noAttrs);
    QVERIFY(sres.size() == 0);

    sres = coll2->searchItems("", noAttrs);
    QVERIFY(sres.size() == 0);

    sres = coll2->searchItems(nullptr);
    QVERIFY(sres.size() == 0);

    sres = coll2->searchItems(nullptr, noAttrs);
    QVERIFY(sres.size() == 0);
}

void KSecretServiceStoreTest::testCreateCollectionFailOnReadonly()
{
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData());

    createTimeMark = std::time(nullptr);
    auto crval1 = backend.createCollection(collName1);
    QVERIFY(!crval1);
}

void KSecretServiceStoreTest::testDirCollections()
{
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData(), false);

    auto dirres = backend.dirCollections();
    QVERIFY(dirres);
    auto colList = dirres.result_;
    QVERIFY(colList.size() == 2); // we created two collections at testCreateCollection
    QVERIFY(colList[0] == collName1);
    QVERIFY(colList[1] == collName2);
}

void KSecretServiceStoreTest::testReadCollection()
{
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData());

    auto rres = backend.readCollection(collName2);
    QVERIFY(rres);
    auto coll = rres.result_;
    QVERIFY(coll.get() != nullptr);
    QVERIFY(coll->label() == collName2);
}

const char *itemName1 = "item1";
const char *itemName2 = "item2";
const char *itemName3 = "item3";
const char *itemNamesWildcard = "item*";
KSecretsStore::ItemValue emptyValue;

void KSecretServiceStoreTest::testCreateItem()
{
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData(), false);

    auto rres = backend.readCollection(collName1);
    QVERIFY(rres);
    auto coll = rres.result_;
    QVERIFY(coll.get() != nullptr);

    auto sres = coll->dirItems();
    QVERIFY(sres.size() == 0);

    KSecretsStore::AttributesMap emptyAttrs;
    auto item = coll->createItem(itemName1, emptyAttrs, emptyValue);
    QVERIFY(item.get() != nullptr);

    auto cres1 = coll->createItem(itemName1, emptyAttrs, emptyValue);
    QVERIFY(cres1.get() == nullptr); // second time should fail

    QVERIFY(item->label() == collName1);
    QVERIFY(item->value() == emptyValue);
    QVERIFY(item->attributes() == emptyAttrs);

    auto item2 = coll->createItem(itemName2, emptyValue);
    QVERIFY(item2.get() != nullptr);
    QVERIFY(item->label() == itemName2);
    QVERIFY(item->value() == emptyValue);
    QVERIFY(item->attributes() == emptyAttrs);

    std::string testContents = std::string("some test contents");
    KSecretsStore::ItemValue someValue;
    someValue.contentType = "test-data";
    someValue.contents.assign(testContents.begin(), testContents.end());
    auto item3 = coll->createItem(itemName3, someValue);
    QVERIFY(item3.get() != nullptr);
    QVERIFY(item->label() == itemName3);
    QVERIFY(item->value() == someValue);
    QVERIFY(item->attributes() == emptyAttrs);
}

void KSecretServiceStoreTest::testCreateItemFailOnReadonly()
{
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData());

    auto rres = backend.readCollection(collName1);
    QVERIFY(rres);
    auto coll = rres.result_;
    QVERIFY(coll.get() != nullptr);

    KSecretsStore::AttributesMap emptyAttrs;
    auto item = coll->createItem(itemName1, emptyAttrs, emptyValue);
    QVERIFY(item.get() == nullptr);
}

void KSecretServiceStoreTest::testSearchItem()
{
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData());
    // this test expends previous testCreateItem put in some items in collection named collName1
    // so first load that collection
    auto rres = backend.readCollection(collName1);
    QVERIFY(rres);
    auto coll = rres.result_;
    QVERIFY(coll.get() != nullptr);

    auto sres = coll->dirItems();
    QVERIFY(sres.size() == 0);

    // ok, now the test
    auto list1 = coll->searchItems(itemName1);
    QVERIFY(list1.size() == 1); // we used exact match so it should find one item
    QVERIFY(list1.front()->label() == itemName1);

    auto listN = coll->searchItems(itemNamesWildcard);
    QVERIFY(listN.size() == 3); // we should find all 3 items here

    auto list3 = coll->searchItems(itemName3);
    QVERIFY(list3.size() == 1);
    QVERIFY(list1.front()->label() == itemName3);
}

void KSecretServiceStoreTest::testItem() {
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData(), false);
    // this test expends previous testCreateItem put in some items in collection named collName1
    // so first load that collection
    auto rres = backend.readCollection(collName1);
    QVERIFY(rres);
    auto coll = rres.result_;
    QVERIFY(coll.get() != nullptr);

    auto list1 = coll->searchItems(itemName1);
    QVERIFY(list1.size() == 1); // we used exact match so it should find one item
    QVERIFY(list1.front()->label() == itemName1);

    // ok, now the test
    auto item = list1.front();

    QVERIFY(item->createdTime() > createTimeMark);
    QVERIFY(item->modifiedTime() > createTimeMark);
    const char* newLabel1 = "new label 1";
    QVERIFY(item->setLabel(newLabel1));
    QVERIFY(item->label() == newLabel1);

    const char *newContentType = "changed-content-type";
    std::string newContents = "some other contents";
    auto val = item->value();
    val.contentType = newContentType;
    val.contents.assign(newContents.begin(), newContents.end());
    QVERIFY(item->setValue(val));

    auto val1 = item->value();
    QVERIFY(val1 == val);

}

void KSecretServiceStoreTest::testItemModifyFailOnReadonly()
{
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData());
    // this test expends previous testCreateItem put in some items in collection named collName1
    // so first load that collection
    auto rres = backend.readCollection(collName1);
    QVERIFY(rres);
    auto coll = rres.result_;
    QVERIFY(coll.get() != nullptr);

    auto list1 = coll->searchItems(itemName1);
    QVERIFY(list1.size() == 1); // we used exact match so it should find one item
    QVERIFY(list1.front()->label() == itemName1);

    // ok, now the test
    auto item = list1.front();
    QVERIFY(!item->setLabel("dummy"));

    auto val = item->value();
    val.contentType = "new-content-type";
    QVERIFY(!item->setValue(val));

    auto attrs = item->attributes();
    attrs.emplace("test.kde.org", "test attr");
    QVERIFY(!item->setAttributes(attrs));
}

void KSecretServiceStoreTest::testDeleteItem() {
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData(), false);

    auto rres = backend.readCollection(collName1);
    QVERIFY(rres);
    auto coll1 = rres.result_;

    auto dres1 = coll1->dirItems();

    auto il1 = coll1->searchItems(itemName2);
    QVERIFY(il1.size() == 1);
    QVERIFY(coll1->deleteItem(il1.front()));

    auto dres2 = coll1->dirItems();
    QVERIFY(dres2.size() < dres1.size());

    il1 = coll1->searchItems(itemName2);
    QVERIFY(il1.size() == 0);
}

void KSecretServiceStoreTest::testDeleteItemFailOnReadonly() {
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData());

    auto rres = backend.readCollection(collName1);
    QVERIFY(rres);
    auto coll1 = rres.result_;

    auto dres1 = coll1->dirItems();

    auto il1 = coll1->searchItems(itemName2);
    QVERIFY(il1.size() == 1);
    QVERIFY(!coll1->deleteItem(il1.front()));

    auto dres2 = coll1->dirItems();
    QVERIFY(dres2.size() == dres1.size());
}

void KSecretServiceStoreTest::testDeleteCollection()
{
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData(), false);

    auto rres = backend.readCollection(collName1);
    QVERIFY(rres);
    auto coll1 = rres.result_;
    auto dres = backend.deleteCollection(coll1);
    QVERIFY(dres);

    // try to read it again
    rres = backend.readCollection(collName1);
    QVERIFY(!rres);
}

void KSecretServiceStoreTest::testDeleteCollectionFailOnReadonly()
{
    KSecretsStore backend;
    auto setupfut = backend.setup(secretsFilePath.toLocal8Bit().constData());

    auto list1 = backend.dirCollections().result_;

    auto rres = backend.readCollection(collName1);
    QVERIFY(rres);
    auto coll1 = rres.result_;
    auto dres = backend.deleteCollection(coll1);
    QVERIFY(!dres);

    auto list2 = backend.dirCollections().result_;
    QVERIFY(list1 == list2);
}

void KSecretServiceStoreTest::cleanupTestCase() { QDir::home().remove(secretsFilePath); }

// vim: tw=220 ts=4
