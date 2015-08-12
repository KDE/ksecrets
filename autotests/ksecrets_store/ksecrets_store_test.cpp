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

QTEST_GUILESS_MAIN(KSecretServiceStoreTest);

const char* test_file_path = "~/.qttest/ksecretsbackend-test.dat";

KSecretServiceStoreTest::KSecretServiceStoreTest(QObject* parent)
    : QObject(parent)
{
}

void KSecretServiceStoreTest::initTestCase()
{
    // create a test file here and performe the real open test below
    KSecretsStore backend;
    auto setupfut = backend.setup(test_file_path, "test");
}

void KSecretServiceStoreTest::cleanupTestCase()
{
    QDir::home().remove(QLatin1Literal(test_file_path));
}

void KSecretServiceStoreTest::testOpen()
{
    KSecretsStore backend;
    auto setupfut = backend.setup(test_file_path, "test");
    auto openfut = backend.open(true);
    auto openres = openfut.get();
    QVERIFY(openres.status_ == KSecretsStore::StoreStatus::Good);
}
