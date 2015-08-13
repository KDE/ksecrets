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

void KSecretServiceStoreTest::testCreateCollection() { QVERIFY(false); }
void KSecretServiceStoreTest::testCreateItem() { QVERIFY(false); }
void KSecretServiceStoreTest::testSearchItem() { QVERIFY(false); }
void KSecretServiceStoreTest::testItem() { QVERIFY(false); }
void KSecretServiceStoreTest::testDeleteItem() { QVERIFY(false); }
void KSecretServiceStoreTest::testDirCollections() { QVERIFY(false); }
void KSecretServiceStoreTest::testReadCollection() { QVERIFY(false); }
void KSecretServiceStoreTest::testDeleteCollection() { QVERIFY(false); }
void KSecretServiceStoreTest::cleanupTestCase() { QDir::home().remove(secretsFilePath); }

// void KSecretServiceStoreTest::testOpen()
// {
//     KSecretsStore backend;
//     auto setupfut =
//     backend.setup(secretsFilePath.toLocal8Bit().constData());
//     QVERIFY(setupfut.get());
// }

// vim: tw=220 ts=4
