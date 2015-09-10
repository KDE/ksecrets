/*
 * Copyright 2011, Valentin Rusu <valir@kde.org>
 * Copyright 2015, Valentin Rusu <valir@kde.org>
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

#include "ksecretsservice-test.h"
#include <ksecretsservice.h>
#include <ksecretscollection.h>
#include <ksecretsvalue.h>
#include <ksecretsitem.h>
#include <pam_credentials.h>

#include <QtCore/QtGlobal>
#include <QtTest/QTest>
#include <QDebug>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

extern "C" {
#include <keyutils.h>
}

QTEST_GUILESS_MAIN(KSecretServiceTest)

#define TEST_CREATE_COLLECTION_NAME "kf5-create-collection"
#define TEST_COLLECTION_NAME "kf5-test-collection"

KSecretServiceTest::KSecretServiceTest(QObject* parent)
    : QObject(parent)
{
}

KSecrets::CollectionPtr collection;
KSharedConfig::Ptr sharedConfig;
QString secretsFilePath;

void KSecretServiceTest::initTestCase()
{
    /* This test should not be run under your normal session, as it'll may
     * interact with an existing KSecrets set-up and you risk corrupting your
     * main secrets file. On an existing session, this file is already present
     */
    QStandardPaths::setTestModeEnabled(true);
    sharedConfig = KSharedConfig::openConfig(QLatin1String("ksecretsrc"));

    secretsFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QVERIFY(QDir::home().mkpath(secretsFilePath));
    secretsFilePath += QLatin1Literal("/ksecrets-test.data");
    qDebug() << "secrets store path: " << secretsFilePath;

    setupKeyring();

    collection = KSecrets::Service::findCollection(
                     QLatin1String(TEST_CREATE_COLLECTION_NAME),
                     KSecrets::Service::CreateCollection).result();
    QVERIFY(collection->isValid().result());
}

void KSecretServiceTest::testCreateAndDelete()
{
    auto collection = KSecrets::Service::findCollection(
                          QLatin1String(TEST_CREATE_COLLECTION_NAME),
                          KSecrets::Service::OpenOnly).result();
    QCOMPARE(collection->status(), KSecrets::Collection::NotFound);

    collection = KSecrets::Service::findCollection(
                     QLatin1String(TEST_CREATE_COLLECTION_NAME),
                     KSecrets::Service::CreateCollection).result();
    QCOMPARE(collection->status(), KSecrets::Collection::NewlyCreated);

    auto removeResult = collection->deleteCollection().result();
    QVERIFY(removeResult);
}

void KSecretServiceTest::testRenameCollection()
{
    auto NEW_COLLECTION_LABEL = QLatin1Literal("New collection name");
    auto oldLabel = collection->label().result();
    auto renameRes = collection->setLabel(NEW_COLLECTION_LABEL);
    QVERIFY(renameRes);
    auto newName = collection->label().result();
    QCOMPARE(newName, NEW_COLLECTION_LABEL);
}

void KSecretServiceTest::testItems()
{
    auto NEW_ITEM_NAME = QLatin1Literal("Test Item1");
    auto NEW_ITEM_VALUE = QLatin1Literal("highly secret value");
    QDateTime testTime = QDateTime::currentDateTime();

    KSecrets::Secret secret;
    secret.setValue(NEW_ITEM_VALUE);
    auto createRes = collection->createItem(NEW_ITEM_NAME, secret).result();
    QVERIFY(createRes);

    auto foundItems = collection->searchItems(NEW_ITEM_NAME).result();
    QVERIFY(foundItems.length() == 1);

    auto theItem = foundItems.first();
    QCOMPARE(theItem->label().result(), NEW_ITEM_NAME);
    QVERIFY(theItem->createdTime().result() > testTime);
    QVERIFY(theItem->modifiedTime().result() > testTime);

    QDateTime oldModifiedTime = theItem->modifiedTime().result();
    QVERIFY(theItem->setLabel(NEW_ITEM_NAME).result());
    QVERIFY(theItem->modifiedTime().result()
        == oldModifiedTime); // name was the same so item should have stayed
                             // the same

    auto NEW_ITEM_SECOND_NAME = QLatin1Literal("Test Item2");
    QVERIFY(theItem->setLabel(NEW_ITEM_SECOND_NAME).result());
    QCOMPARE(theItem->label().result(), NEW_ITEM_SECOND_NAME);
    QVERIFY(theItem->modifiedTime().result() > oldModifiedTime);

    auto theSecret = theItem->getSecret().result();
    QCOMPARE(theSecret->value().toString(), NEW_ITEM_VALUE);
}


void KSecretServiceTest::cleanupTestCase()
{
    if (collection->isValid().result()) {
        collection->deleteCollection();
    }
    cleanupKeyring();

    QString configPath = QStandardPaths::locate(
            QStandardPaths::ConfigLocation,
            sharedConfig->name());
    qDebug() << "removing config file " << configPath;
    QFile(configPath).remove();
}


void KSecretServiceTest::setupKeyring()
{
    const char* KSS_TEST_USER = "USER";
    QVERIFY(qEnvironmentVariableIsSet(KSS_TEST_USER));
    QVERIFY(!qEnvironmentVariableIsEmpty(KSS_TEST_USER));
    auto testUser = qgetenv(KSS_TEST_USER);

    const char* KSS_TEST_PASSWORD = "KSS_TEST_PASSWORD";
    QVERIFY(qEnvironmentVariableIsSet(KSS_TEST_PASSWORD));
    QVERIFY(!qEnvironmentVariableIsEmpty(KSS_TEST_PASSWORD));
    auto testPass = qgetenv(KSS_TEST_PASSWORD);

    const char* KEYNAME_ENCRYPTING = "ksecrets-test:encrypting";
    const char* KEYNAME_MAC = "ksecrets-test:mac";

    /* second prevention measure - change keyring keynames */
    sharedConfig->group(QLatin1String("keyring"))
        .writeEntry("keyname_encrypt", KEYNAME_ENCRYPTING);
    sharedConfig->group(QLatin1String("keyring"))
        .writeEntry("keyname_mac", KEYNAME_MAC);

    /* third prevention measure - the encrypting key should not be present
     * into the kernel keyring */
    key_serial_t key;
    key = request_key("user", KEYNAME_ENCRYPTING, 0, KEY_SPEC_SESSION_KEYRING);
    QVERIFY(-1 == key);

    /* now go setup user's keyring */
    QVERIFY(1 == kss_set_credentials(testUser.constData(), testPass.constData(), secretsFilePath.toLocal8Bit().constData()));

    // the right keys should be present into the kernel keyring
    key = request_key("user", KEYNAME_ENCRYPTING, 0, KEY_SPEC_SESSION_KEYRING);
    QVERIFY(-1 != key);
    key = request_key("user", KEYNAME_MAC, 0, KEY_SPEC_SESSION_KEYRING);
    QVERIFY(-1 != key);
}

void KSecretServiceTest::cleanupKeyring()
{
    QVERIFY(kss_delete_credentials());
}

#include "ksecretsservice-test.moc"
