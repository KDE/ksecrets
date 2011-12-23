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

#include "dialoguimanagertest.h"

#include <QtCrypto>
#include <qtest_kde.h>

#include "../dialoguimanager.h"

void DialogUiManagerTest::initTestCase()
{
    QCA::init();
}

void DialogUiManagerTest::testAskPassword()
{
    DialogUiManager fact;

    // ask for a password (asynchronously)
    AbstractAskPasswordJob *asyncJob1 = fact.createAskPasswordJob("TESTCOLLECTION-ASYNC1", false);
    AbstractAskPasswordJob *asyncJob2 = fact.createAskPasswordJob("TESTCOLLECTION-ASYNC2", false);
    QEventLoop loop;
    connect(asyncJob2, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
    asyncJob1->start();
    asyncJob2->start();
    loop.exec();
}

void DialogUiManagerTest::testNewPassword()
{
    DialogUiManager fact;

    // ask for a new password (asynchronously)
    AbstractNewPasswordJob *asyncJob1 = fact.createNewPasswordJob("TESTCOLLECTION");
    QEventLoop loop;
    connect(asyncJob1, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
    asyncJob1->start();
    loop.exec();
}

void DialogUiManagerTest::cleanupTestCase()
{
}

QTEST_KDEMAIN(DialogUiManagerTest, GUI)
#include "dialoguimanagertest.moc"
