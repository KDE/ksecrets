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

#ifndef TEMPORARYTEST_H
#define TEMPORARYTEST_H

#include <QtTest/QtTest>

class BackendCollectionManager;
class BackendMaster;

class TemporaryTest : public QObject
{
    Q_OBJECT
public:
    TemporaryTest(QObject* parent = 0);
    virtual ~TemporaryTest();

private Q_SLOTS:
    void initTestCase();

    void testCreateCollectionSync();
    void testCreateItemSync();
    void testReplaceItemSync();
    void testDoNotReplaceItemSync();
    void testDeleteItemSync();
    void testDeleteCollectionSync();

    void testCreateCollectionAsync();
    void testCreateItemAsync();
    void testReplaceItemAsync();
    void testDoNotReplaceItemAsync();
    void testDeleteItemAsync();
    void testDeleteCollectionAsync();

    void cleanupTestCase();

private:
    BackendMaster *m_master;
    BackendCollectionManager *m_manager;
};

#endif // TEMPORARYTEST_H
