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

#include "ksecrets_file_test.h"

#include <ksecrets_file.h>
#include <ksecrets_store.h>
#include <QtTest/QtTest>
#include <stdlib.h>

QTEST_GUILESS_MAIN(KSecretsFileTest)

KSecretsFileTest::KSecretsFileTest() {}
KSecretsFileTest::~KSecretsFileTest() {}

void KSecretsFileTest::initTestCase()
{
    KSecretsStore backend;
    auto credfut = backend.setCredentials("test", "ksecrets-test:crypt", "ksecrets-test:mac");
    QVERIFY(credfut.get());
}

void KSecretsFileTest::testIntegrityCheck()
{
    const char* TEST_FILE_NAME = "ksecrets_file_test_tmp.data";

    KSecretsFile theFile;
    theFile.create(TEST_FILE_NAME);
    theFile.setup(TEST_FILE_NAME, false);
    QVERIFY(theFile.openAndCheck() == KSecretsFile::OpenStatus::Ok);
}
// vim: tw=220:ts=4
