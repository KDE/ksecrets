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

#include "servicetest.h"

#include "backend/backendmaster.h"
#include "backend/temporary/temporarycollectionmanager.h"
#include "tempblockingcollectionmanager.h"
#include "secret/service.h"
#include "secret/adaptors/dbustypes.h"
#include "ui/nouimanager.h"

#include <qtest_kde.h>

#include <QtTest/QTest>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <QtCrypto>

#include <QtCore/QDebug>
#include "../secret/adaptors/dbustypes.h"
#include <kdebug.h>

/*
 * NOTE the DBUS_SERVICE here is purposely not the same as the name
 * used by the daemon, as theses tests must be performed locally
 * and not over the dbus and calling an eventually running daemon
 */
#define DBUS_SERVICE "org.freedesktop.secrets.frontend.test"

void ServiceTest::initTestCase()
{
    QVERIFY(QDBusConnection::sessionBus().registerService( DBUS_SERVICE ));

    QCA::init();

    m_master = BackendMaster::instance();
    m_master->setUiManager(new NoUiManager);
    m_tempCollMan = new TemporaryCollectionManager(m_master);
    m_master->addManager(m_tempCollMan);
    m_service = new Service(m_master);
}

void ServiceTest::dbusService()
{
    // make sure the service is available
    QDBusConnectionInterface *ifaceConn = QDBusConnection::sessionBus().interface();
    QVERIFY(ifaceConn && ifaceConn->isValid());

    QDBusReply<bool> registered = ifaceConn->isServiceRegistered( DBUS_SERVICE );
    QVERIFY(registered.isValid());
    QVERIFY(registered.value());
}

void ServiceTest::session()
{
    QDBusInterface ifaceService( DBUS_SERVICE, "/org/freedesktop/secrets");
    QVERIFY(ifaceService.isValid());
    QCA::KeyGenerator keygen;

    // "unsupported" session algorithm
    QList<QVariant> unsuppInput;
    unsuppInput << QString("unsupported") << QVariant::fromValue(QDBusVariant(QString("")));
    QDBusMessage unsuppReply = ifaceService.callWithArgumentList(QDBus::Block, "OpenSession",
                               unsuppInput);
    QCOMPARE(unsuppReply.type(), QDBusMessage::ErrorMessage);
//    kDebug() << unsuppReply.errorName();
//    QEXPECT_FAIL("", "Error only returned if called via D-Bus (test calls locally)", Continue);
    QCOMPARE(unsuppReply.errorName(),
             QLatin1String("org.freedesktop.Secret.Error.NotSupported"));

    // "plain" session algorithm
    QDBusObjectPath plainPath;
    QList<QVariant> plainInput;
    plainInput << QString("plain") << QVariant::fromValue(QDBusVariant(""));
    QDBusMessage plainReply = ifaceService.callWithArgumentList(QDBus::Block, "OpenSession",
                              plainInput);
    QCOMPARE(plainReply.type(), QDBusMessage::ReplyMessage);
    QList<QVariant> plainArgs = plainReply.arguments();
    QCOMPARE(plainArgs.size(), 2);
    QCOMPARE(plainArgs.at(0).userType(), qMetaTypeId<QDBusVariant>());
    QCOMPARE(plainArgs.at(0).value<QString>(), QLatin1String(""));
    QCOMPARE(plainArgs.at(1).userType(), qMetaTypeId<QDBusObjectPath>());
    plainPath = plainArgs.at(1).value<QDBusObjectPath>();
    QVERIFY(plainPath.path().startsWith(QLatin1String("/org/freedesktop/secrets/session/")));
    QDBusInterface plainIface( DBUS_SERVICE, plainPath.path(),
                              "org.freedesktop.Secret.Session");
    QVERIFY(plainIface.isValid());
    QDBusMessage plainReply2 = plainIface.call("Close");
    QCOMPARE(plainReply2.type(), QDBusMessage::ReplyMessage);
    QCOMPARE(plainIface.call("Introspect").type(), QDBusMessage::ErrorMessage);

    // "dh-ietf1024-aes128-cbc-pkcs7" encryption
    QDBusObjectPath dhPath;
    QList<QVariant> dhInput;
    QCA::DLGroup dhDlgroup(keygen.createDLGroup(QCA::IETF_1024));
    QVERIFY(!dhDlgroup.isNull());
    QCA::PrivateKey dhPrivkey(keygen.createDH(dhDlgroup));
    QCA::PublicKey dhPubkey(dhPrivkey);
    QByteArray dhBytePub(dhPubkey.toDH().y().toArray().toByteArray());
    dhInput << QString("dh-ietf1024-aes128-cbc-pkcs7")
            << QVariant::fromValue(QDBusVariant(dhBytePub));
    QDBusMessage dhReply = ifaceService.callWithArgumentList(QDBus::Block, "OpenSession",
                           dhInput);
    QCOMPARE(dhReply.type(), QDBusMessage::ReplyMessage);
    QList<QVariant> dhArgs = dhReply.arguments();
    QCOMPARE(dhArgs.size(), 2);
    QCOMPARE(dhArgs.at(0).userType(), qMetaTypeId<QDBusVariant>());
    QVariant dhOutputVar = dhArgs.at(0).value<QDBusVariant>().variant();
    QCOMPARE(dhOutputVar.type(), QVariant::ByteArray);
    QByteArray dhOutput = dhOutputVar.toByteArray();
    QCA::DHPublicKey dhServiceKey(dhDlgroup,
                                  QCA::BigInteger(QCA::SecureArray(dhOutput)));
    QCA::SymmetricKey dhSharedKey(dhPrivkey.deriveKey(dhServiceKey));
    QCA::Cipher *dhCipher = new QCA::Cipher("aes128", QCA::Cipher::CBC, QCA::Cipher::PKCS7);
    QVERIFY(dhSharedKey.size() >= dhCipher->keyLength().minimum());
    QCOMPARE(plainArgs.at(1).userType(), qMetaTypeId<QDBusObjectPath>());
    dhPath = dhArgs.at(1).value<QDBusObjectPath>();
    QVERIFY(dhPath.path().startsWith(QLatin1String("/org/freedesktop/secrets/session/")));
    QDBusInterface dhIface( DBUS_SERVICE, dhPath.path(),
                           "org.freedesktop.Secret.Session");
    QVERIFY(dhIface.isValid());
    QDBusMessage dhReply2 = dhIface.call("Close");
    QCOMPARE(dhReply2.type(), QDBusMessage::ReplyMessage);
    QCOMPARE(dhIface.call("Introspect").type(), QDBusMessage::ErrorMessage);
}

void ServiceTest::nonBlockingCollection()
{
    QDBusInterface ifaceService(DBUS_SERVICE, "/org/freedesktop/secrets");
    QVERIFY(ifaceService.isValid());

    // create a session
    QDBusObjectPath sessionPath;
    QList<QVariant> sessionInput;
    sessionInput << QString("plain") << QVariant::fromValue(QDBusVariant(""));
    QDBusMessage sessionReply = ifaceService.callWithArgumentList(QDBus::Block, "OpenSession",
                                sessionInput);
    sessionPath = sessionReply.arguments().at(1).value<QDBusObjectPath>();

    // listen to CollectionCreated/CollectionDeleted/CollectionChanged signals
    ObjectPathSignalSpy createdSpy(&ifaceService, SIGNAL(CollectionCreated(QDBusObjectPath)));
    QVERIFY(createdSpy.isValid());
    ObjectPathSignalSpy deletedSpy(&ifaceService, SIGNAL(CollectionDeleted(QDBusObjectPath)));
    QVERIFY(deletedSpy.isValid());
    ObjectPathSignalSpy changedSpy(&ifaceService, SIGNAL(CollectionChanged(QDBusObjectPath)));
    QVERIFY(changedSpy.isValid());

    // create a collection
    QDBusObjectPath collectionPath;
    StringVariantMap createProperties;
    QList<QVariant> createInput;
    createProperties["Label"] = "test";
    createProperties["Locked"] = false; // create collection unlocked
    createInput << QVariant::fromValue(createProperties);
    createInput << QString("test alias");
    QDBusMessage createReply = ifaceService.callWithArgumentList(QDBus::Block, "CreateCollection",
                               createInput);

    QCOMPARE(createReply.type(), QDBusMessage::ReplyMessage);
    QList<QVariant> createArgs = createReply.arguments();
    QCOMPARE(createArgs.size(), 2);
    QCOMPARE(createArgs.at(0).userType(), qMetaTypeId<QDBusObjectPath>());
    QCOMPARE(createArgs.at(1).userType(), qMetaTypeId<QDBusObjectPath>());
    // CreateCollection is blocking, so the first output (path) should be "/".
    QCOMPARE(createArgs.at(0).value<QDBusObjectPath>().path(), QLatin1String("/"));
    QDBusObjectPath promptPath = createArgs.at(1).value<QDBusObjectPath>();
    QVERIFY(promptPath.path().startsWith(
                QLatin1String("/org/freedesktop/secrets/prompts/")));

    // prompt and wait for the result.
    ClientPrompt *prompt = new ClientPrompt(promptPath);
    prompt->promptAndWait(5000);
    QVERIFY(prompt->completed());
    QVERIFY(!prompt->dismissed());
    QCOMPARE(prompt->result().userType(), qMetaTypeId<QDBusObjectPath>());
    collectionPath = prompt->result().value<QDBusObjectPath>();
    QVERIFY(collectionPath.path().startsWith(
                QLatin1String("/org/freedesktop/secrets/collection/")));
    QDBusInterface ifaceCollection(DBUS_SERVICE, collectionPath.path(),
                                   "org.freedesktop.Secret.Collection");
    QVERIFY(ifaceCollection.isValid());
    delete prompt;

    // make sure the CollectionCreated signal was sent
    if(createdSpy.size() < 1) {
        createdSpy.waitForSignal(5000);
    }
    QCOMPARE(createdSpy.size(), 1);
    QCOMPARE(createdSpy.takeFirst(), collectionPath);

    // read collection properties
    QVariant propItems = ifaceCollection.property("Items");
    QVERIFY(propItems.isValid());
    QVERIFY(propItems.canConvert<QList<QDBusObjectPath> >());
    QList<QDBusObjectPath> propItemsList = propItems.value<QList<QDBusObjectPath> >();
    QVERIFY(propItemsList.isEmpty());
    QVariant propLabel = ifaceCollection.property("Label");
    QVERIFY(propLabel.isValid());
    QCOMPARE(propLabel.type(), QVariant::String);
    QCOMPARE(propLabel.value<QString>(), QString("test alias"));
    QVariant propLocked = ifaceCollection.property("Locked");
    QVERIFY(propLocked.isValid());
    QCOMPARE(propLocked.value<bool>(), false);
    QVariant propCreated = ifaceCollection.property("Created");
    QVERIFY(propCreated.isValid());
    QCOMPARE(propCreated.type(), QVariant::ULongLong);
    qulonglong propCreatedUll = propCreated.value<qulonglong>();
    QVERIFY(QDateTime::currentDateTime().toTime_t() - propCreatedUll < 60);
    QVariant propModified = ifaceCollection.property("Modified");
    QVERIFY(propModified.isValid());
    QCOMPARE(propModified.type(), QVariant::ULongLong);
    QCOMPARE(propModified.value<qulonglong>(), propCreatedUll);

    // set the label and re-read it.
    ifaceCollection.setProperty("Label", QString("test2"));
    propLabel = ifaceCollection.property("Label");
    QVERIFY(propLabel.isValid());
    QCOMPARE(propLabel.type(), QVariant::String);
    QCOMPARE(propLabel.value<QString>(), QString("test2"));

    // make sure the CollectionChanged signal was sent
    if(changedSpy.size() < 1) {
        changedSpy.waitForSignal(5000);
    }
    QCOMPARE(changedSpy.size(), 1);
    QCOMPARE(changedSpy.takeFirst(), collectionPath);

    // delete the collection
    QDBusMessage deleteReply = ifaceCollection.call(QDBus::Block, "Delete");
    QCOMPARE(deleteReply.type(), QDBusMessage::ReplyMessage);
    QList<QVariant> deleteArgs = deleteReply.arguments();
    QCOMPARE(deleteArgs.size(), 1);
    QCOMPARE(deleteArgs.at(0).userType(), qMetaTypeId<QDBusObjectPath>());
    // TemporaryCollection is non-blocking, so the output (prompt) should be "/".
    QCOMPARE(deleteArgs.at(0).value<QDBusObjectPath>().path(), QLatin1String("/"));
    // make sure the collection is gone
    QCOMPARE(ifaceCollection.call("Introspect").type(), QDBusMessage::ErrorMessage);

    // make sure the CollectionDeleted signal was sent
    if(deletedSpy.size() < 1) {
        deletedSpy.waitForSignal(5000);
    }
    QCOMPARE(deletedSpy.size(), 1);
    QCOMPARE(deletedSpy.takeFirst(), collectionPath);

    // close the session
    QDBusInterface(DBUS_SERVICE, sessionPath.path()).call("Close");
}

void ServiceTest::nonBlockingItem()
{
    QDBusInterface ifaceService(DBUS_SERVICE, "/org/freedesktop/secrets");

    // create a session
    QDBusObjectPath sessionPath;
    QList<QVariant> sessionInput;
    sessionInput << QString("plain") << QVariant::fromValue(QDBusVariant(""));
    QDBusMessage sessionReply = ifaceService.callWithArgumentList(QDBus::Block, "OpenSession",
                                sessionInput);
    sessionPath = sessionReply.arguments().at(1).value<QDBusObjectPath>();

    // create a collection
    QDBusObjectPath collectionPath;
    QMap<QString, QVariant> collProperties;
    QList<QVariant> collInput;
    collProperties["Label"] = "test3";
    collProperties["Locked"] = false;
    collInput << QVariant::fromValue(collProperties);
    collInput << "test alias";
    QDBusMessage collReply = ifaceService.callWithArgumentList(QDBus::Block, "CreateCollection",
                             collInput);
    QDBusObjectPath promptPath = collReply.arguments().at(1).value<QDBusObjectPath>();
    ClientPrompt *prompt = new ClientPrompt(promptPath);
    prompt->promptAndWait(5000);
    QVERIFY(prompt->completed());
    collectionPath = prompt->result().value<QDBusObjectPath>();
    delete prompt;
    QDBusInterface ifaceColl(DBUS_SERVICE, collectionPath.path());

    ObjectPathSignalSpy createdSpy(&ifaceColl, SIGNAL(ItemCreated(QDBusObjectPath)));
    QVERIFY(createdSpy.isValid());
    ObjectPathSignalSpy deletedSpy(&ifaceColl, SIGNAL(ItemDeleted(QDBusObjectPath)));
    QVERIFY(deletedSpy.isValid());
    ObjectPathSignalSpy changedSpy(&ifaceColl, SIGNAL(ItemChanged(QDBusObjectPath)));
    QVERIFY(changedSpy.isValid());

    // create an item
    QDBusObjectPath itemPath;
    StringStringMap itemAttributes;
    itemAttributes["attribute1"] = "value1";
    itemAttributes["attribute2"] = "value2";
    QMap<QString, QVariant> itemProperties;
    itemProperties["Attributes"] = QVariant::fromValue(itemAttributes);
    itemProperties["Label"] = "item1";
    itemProperties["Locked"] = false;
    QList<QVariant> itemInput;
    SecretStruct secret;
    secret.m_session = sessionPath;
    secret.m_value = QByteArray("mysecret");
    itemInput << QVariant::fromValue(itemProperties);
    itemInput << QVariant::fromValue(secret);
    itemInput << false;
    QDBusMessage itemReply = ifaceColl.callWithArgumentList(QDBus::Block, "CreateItem",
                             itemInput);
    kDebug() << itemReply.errorMessage();
    QCOMPARE(itemReply.type(), QDBusMessage::ReplyMessage);
    QList<QVariant> itemArgs = itemReply.arguments();
    QCOMPARE(itemArgs.size(), 2);
    QCOMPARE(itemArgs.at(0).userType(), qMetaTypeId<QDBusObjectPath>());
    QCOMPARE(itemArgs.at(1).userType(), qMetaTypeId<QDBusObjectPath>());
    // TemporaryItem is non-blocking, so the second output (prompt) should be "/"
    QCOMPARE(itemArgs.at(1).value<QDBusObjectPath>().path(), QLatin1String("/"));
    itemPath = itemArgs.at(0).value<QDBusObjectPath>();
    QVERIFY(itemPath.path().startsWith(collectionPath.path() + '/'));
    QDBusInterface ifaceItem(DBUS_SERVICE, itemPath.path(),
                             "org.freedesktop.Secret.Item");
    QVERIFY(ifaceItem.isValid());

    // make sure the ItemCreated signal was sent
    if(createdSpy.size() < 1) {
        createdSpy.waitForSignal(5000);
    }
    QCOMPARE(createdSpy.size(), 1);
    QCOMPARE(createdSpy.takeFirst(), itemPath);

    // check if the collection's Items property contains the item.
    QVariant propItems = ifaceColl.property("Items");
    QVERIFY(propItems.isValid());
    QVERIFY(propItems.canConvert<QList<QDBusObjectPath> >());
    QList<QDBusObjectPath> propItemsList = propItems.value<QList<QDBusObjectPath> >();
    QCOMPARE(propItemsList.count(), 1);
    QCOMPARE(propItemsList.at(0), itemPath);

    // read item properties
    QVariant propLocked = ifaceItem.property("Locked");
    QVERIFY(propLocked.isValid());
    QCOMPARE(propLocked.type(), QVariant::Bool);
    QCOMPARE(propLocked.value<bool>(), false);
    QVariant propLabel = ifaceItem.property("Label");
    QVERIFY(propLabel.isValid());
    QCOMPARE(propLabel.type(), QVariant::String);
    QCOMPARE(propLabel.value<QString>(), QLatin1String("item1"));
    QVariant propCreated = ifaceItem.property("Created");
    QVERIFY(propCreated.isValid());
    QCOMPARE(propCreated.type(), QVariant::ULongLong);
    qulonglong propCreatedUll = propCreated.value<qulonglong>();
    QVERIFY(QDateTime::currentDateTime().toTime_t() - propCreatedUll < 60);
    QVariant propModified = ifaceItem.property("Modified");
    QVERIFY(propModified.isValid());
    QCOMPARE(propModified.type(), QVariant::ULongLong);
    QCOMPARE(propModified.value<qulonglong>(), propCreatedUll);
    QVariant propAttributes = ifaceItem.property("Attributes");
    QVERIFY(propAttributes.isValid());
    QCOMPARE(propAttributes.userType(), qMetaTypeId<StringStringMap>());
    StringStringMap propAttributesMap = propAttributes.value<StringStringMap>();
    QCOMPARE(propAttributesMap.size(), 3); // the Label attribute is automatically added when creating the new item
    QCOMPARE(propAttributesMap.value("attribute1"), QLatin1String("value1"));
    QCOMPARE(propAttributesMap.value("attribute2"), QLatin1String("value2"));

    // read the secret
    QDBusMessage secretReply = ifaceItem.call(QDBus::Block, "GetSecret",
                               QVariant::fromValue<QDBusObjectPath>(sessionPath));
    QCOMPARE(secretReply.type(), QDBusMessage::ReplyMessage);
    QList<QVariant> secretOutput = secretReply.arguments();
    QCOMPARE(secretOutput.count(), 1);
    SecretStruct outsecret = qdbus_cast<SecretStruct>(secretOutput.at(0));
    QCOMPARE(outsecret.m_session, sessionPath);
    QCOMPARE(outsecret.m_value, QByteArray("mysecret"));

    // set and re-read item properties
    ifaceItem.setProperty("Label", QString("item2"));
    propLabel = ifaceItem.property("Label");
    QVERIFY(propLabel.isValid());
    QCOMPARE(propLabel.type(), QVariant::String);
    QCOMPARE(propLabel.value<QString>(), QLatin1String("item2"));
    propAttributesMap["attribute3"] = "value3";
    propAttributesMap["attribute1"] = "othervalue";
    propAttributesMap.remove("attribute2");
    ifaceItem.setProperty("Attributes", QVariant::fromValue<StringStringMap>(propAttributesMap));
    propAttributes = ifaceItem.property("Attributes");
    QVERIFY(propAttributes.isValid());
    QCOMPARE(propAttributes.userType(), qMetaTypeId<StringStringMap>());
    propAttributesMap = propAttributes.value<StringStringMap>();
    QCOMPARE(propAttributesMap.size(), 3); // Label attribute is automatically created when creating the item
    QCOMPARE(propAttributesMap.value("attribute1"), QLatin1String("othervalue"));
    QCOMPARE(propAttributesMap.value("attribute3"), QLatin1String("value3"));

    // set and re-read the secret
    secret.m_value = "mysecret2";
    secretReply = ifaceItem.call(QDBus::Block, "SetSecret", QVariant::fromValue<SecretStruct>(secret));
    QCOMPARE(secretReply.type(), QDBusMessage::ReplyMessage);
    secretReply = ifaceItem.call(QDBus::Block, "GetSecret",
                                 QVariant::fromValue<QDBusObjectPath>(sessionPath));
    QCOMPARE(secretReply.type(), QDBusMessage::ReplyMessage);
    secretOutput = secretReply.arguments();
    QCOMPARE(secretOutput.count(), 1);
    outsecret = qdbus_cast<SecretStruct>(secretOutput.at(0));
    QCOMPARE(outsecret.m_session, sessionPath);
    QCOMPARE(outsecret.m_value, QByteArray("mysecret2"));


    // we should have received 4 ItemChanged signals
    if(changedSpy.size() < 4) {
        changedSpy.waitForSignals(4, 5000);
    }
    QCOMPARE(changedSpy.size(), 4);
    QCOMPARE(changedSpy.takeFirst(), itemPath);
    QCOMPARE(changedSpy.takeFirst(), itemPath);
    QCOMPARE(changedSpy.takeFirst(), itemPath);

    // delete the item
    QDBusMessage deleteReply = ifaceItem.call(QDBus::Block, "Delete");
    QCOMPARE(deleteReply.type(), QDBusMessage::ReplyMessage);
    QList<QVariant> deleteArgs = deleteReply.arguments();
    QCOMPARE(deleteArgs.size(), 1);
    QCOMPARE(deleteArgs.at(0).userType(), qMetaTypeId<QDBusObjectPath>());
    // TemporaryItem is non-blocking, so the output (prompt) should be "/"
    QCOMPARE(deleteArgs.at(0).value<QDBusObjectPath>().path(), QLatin1String("/"));
    // make sure the item is gone
    QCOMPARE(ifaceItem.call("Introspect").type(), QDBusMessage::ErrorMessage);

    // make sure the ItemDeleted signal was sent
    if(deletedSpy.size() < 1) {
        deletedSpy.waitForSignal(5000);
    }
    QCOMPARE(deletedSpy.size(), 1);
    QCOMPARE(deletedSpy.takeFirst(), itemPath);

    // delete the collection
    ifaceColl.call(QDBus::Block, "Delete");
    // close the session
    QDBusInterface(DBUS_SERVICE, sessionPath.path()).call("Close");
}

void ServiceTest::reInitTestCase()
{
    // remove the TemporaryCollectionManager and replace it with a TempBlockingCollectionManager
    m_master->removeManager(m_tempCollMan);
    delete m_tempCollMan;
    m_tempBlockCollMan = new TempBlockingCollectionManager(m_master);
    m_master->addManager(m_tempBlockCollMan);
}

void ServiceTest::blockingCollection()
{
    ClientPrompt *prompt = 0;

    QDBusInterface ifaceService(DBUS_SERVICE, "/org/freedesktop/secrets");
    QVERIFY(ifaceService.isValid());

    // create a session
    // create a session
    QDBusObjectPath sessionPath;
    QList<QVariant> sessionInput;
    sessionInput << QString("plain") << QVariant::fromValue(QDBusVariant(""));
    QDBusMessage sessionReply = ifaceService.callWithArgumentList(QDBus::Block, "OpenSession",
                                sessionInput);
    sessionPath = sessionReply.arguments().at(1).value<QDBusObjectPath>();

    // listen to CollectionCreated/CollectionDeleted/CollectionChanged signals
    ObjectPathSignalSpy createdSpy(&ifaceService, SIGNAL(CollectionCreated(QDBusObjectPath)));
    QVERIFY(createdSpy.isValid());
    ObjectPathSignalSpy deletedSpy(&ifaceService, SIGNAL(CollectionDeleted(QDBusObjectPath)));
    QVERIFY(deletedSpy.isValid());
    ObjectPathSignalSpy changedSpy(&ifaceService, SIGNAL(CollectionChanged(QDBusObjectPath)));
    QVERIFY(changedSpy.isValid());

    // create a collection
    QDBusObjectPath promptPath;
    QMap<QString, QVariant> createProperties;
    QList<QVariant> createInput;
    createProperties["Label"] = "test";
    createProperties["Locked"] = false; // create collection unlocked
    createInput << QVariant::fromValue(createProperties);
    createInput << QString("test alias");
    QDBusMessage createReply = ifaceService.callWithArgumentList(QDBus::Block, "CreateCollection",
                               createInput);
    QCOMPARE(createReply.type(), QDBusMessage::ReplyMessage);
    QList<QVariant> createArgs = createReply.arguments();
    QCOMPARE(createArgs.size(), 2);
    QCOMPARE(createArgs.at(0).userType(), qMetaTypeId<QDBusObjectPath>());
    QCOMPARE(createArgs.at(1).userType(), qMetaTypeId<QDBusObjectPath>());
    // TempBlockingCollection is blocking, so the first output (path) should be "/".
    QCOMPARE(createArgs.at(0).value<QDBusObjectPath>().path(), QLatin1String("/"));
    promptPath = createArgs.at(1).value<QDBusObjectPath>();
    QVERIFY(promptPath.path().startsWith(
                QLatin1String("/org/freedesktop/secrets/prompts/")));

    // dismiss the prompt and wait for the result.
    prompt = new ClientPrompt(promptPath);
    prompt->dismissAndWait(5000);
    QVERIFY(prompt->completed());
    QVERIFY(prompt->dismissed());
    delete prompt;

    createReply = ifaceService.callWithArgumentList(QDBus::Block, "CreateCollection",
                  createInput);
    QCOMPARE(createReply.type(), QDBusMessage::ReplyMessage);
    createArgs = createReply.arguments();
    QCOMPARE(createArgs.size(), 2);
    QCOMPARE(createArgs.at(0).userType(), qMetaTypeId<QDBusObjectPath>());
    QCOMPARE(createArgs.at(1).userType(), qMetaTypeId<QDBusObjectPath>());
    // TempBlockingCollection is blocking, so the first output (path) should be "/".
    QCOMPARE(createArgs.at(0).value<QDBusObjectPath>().path(), QLatin1String("/"));
    promptPath = createArgs.at(1).value<QDBusObjectPath>();
    QVERIFY(promptPath.path().startsWith(
                QLatin1String("/org/freedesktop/secrets/prompts/")));

    // prompt and wait for the result.
    prompt = new ClientPrompt(promptPath);
    prompt->promptAndWait(5000);
    QVERIFY(prompt->completed());
    QVERIFY(!prompt->dismissed());
    QCOMPARE(prompt->result().userType(), qMetaTypeId<QDBusObjectPath>());
    QDBusObjectPath collectionPath = prompt->result().value<QDBusObjectPath>();
    QVERIFY(collectionPath.path().startsWith(
                QLatin1String("/org/freedesktop/secrets/collection/")));
    QDBusInterface ifaceCollection(DBUS_SERVICE, collectionPath.path(),
                                   "org.freedesktop.Secret.Collection");
    QVERIFY(ifaceCollection.isValid());
    delete prompt;

    // make sure the CollectionCreated signal was sent
    if(createdSpy.size() < 1) {
        createdSpy.waitForSignal(5000);
    }
    QCOMPARE(createdSpy.size(), 1);
    QCOMPARE(createdSpy.takeFirst(), collectionPath);

    // read collection properties
    QVariant propItems = ifaceCollection.property("Items");
    QVERIFY(propItems.isValid());
    QVERIFY(propItems.canConvert<QList<QDBusObjectPath> >());
    QList<QDBusObjectPath> propItemsList = propItems.value<QList<QDBusObjectPath> >();
    QVERIFY(propItemsList.isEmpty());
    QVariant propLabel = ifaceCollection.property("Label");
    QVERIFY(propLabel.isValid());
    QCOMPARE(propLabel.type(), QVariant::String);
    QCOMPARE(propLabel.value<QString>(), QString("test alias"));
    QVariant propLocked = ifaceCollection.property("Locked");
    QVERIFY(propLocked.isValid());
    QCOMPARE(propLocked.value<bool>(), false);
    QVariant propCreated = ifaceCollection.property("Created");
    QVERIFY(propCreated.isValid());
    QCOMPARE(propCreated.type(), QVariant::ULongLong);
    qulonglong propCreatedUll = propCreated.value<qulonglong>();
    QVERIFY(QDateTime::currentDateTime().toTime_t() - propCreatedUll < 60);
    QVariant propModified = ifaceCollection.property("Modified");
    QVERIFY(propModified.isValid());
    QCOMPARE(propModified.type(), QVariant::ULongLong);
    QCOMPARE(propModified.value<qulonglong>(), propCreatedUll);

    // set the label and re-read it.
    ifaceCollection.setProperty("Label", QString("test2"));
    propLabel = ifaceCollection.property("Label");
    QVERIFY(propLabel.isValid());
    QCOMPARE(propLabel.type(), QVariant::String);
    QCOMPARE(propLabel.value<QString>(), QString("test2"));

    // make sure the CollectionChanged signal was sent
    if(changedSpy.size() < 1) {
        changedSpy.waitForSignal(5000);
    }
    QCOMPARE(changedSpy.size(), 1);
    QCOMPARE(changedSpy.takeFirst(), collectionPath);

    // delete the collection
    QDBusMessage deleteReply = ifaceCollection.call(QDBus::Block, "Delete");
    QCOMPARE(deleteReply.type(), QDBusMessage::ReplyMessage);
    QList<QVariant> deleteArgs = deleteReply.arguments();
    QCOMPARE(deleteArgs.size(), 1);
    QCOMPARE(deleteArgs.at(0).userType(), qMetaTypeId<QDBusObjectPath>());
    // TempBlockingCollection is blocking, so the output (prompt) should be a valid one.
    promptPath = deleteArgs.at(0).value<QDBusObjectPath>();
    QVERIFY(promptPath.path().startsWith(QLatin1String("/org/freedesktop/secrets/prompts/")));

    // dismiss the prompt and wait for the result.
    prompt = new ClientPrompt(promptPath);
    prompt->dismissAndWait(5000);
    QVERIFY(prompt->completed());
    QVERIFY(prompt->dismissed());
    delete prompt;

    // retry and complete the prompt this time
    deleteReply = ifaceCollection.call(QDBus::Block, "Delete");
    QCOMPARE(deleteReply.type(), QDBusMessage::ReplyMessage);
    deleteArgs = deleteReply.arguments();
    QCOMPARE(deleteArgs.size(), 1);
    QCOMPARE(deleteArgs.at(0).userType(), qMetaTypeId<QDBusObjectPath>());
    // TempBlockingCollection is blocking, so the output (prompt) should be a valid one.
    promptPath = deleteArgs.at(0).value<QDBusObjectPath>();
    QVERIFY(promptPath.path().startsWith(QLatin1String("/org/freedesktop/secrets/prompts/")));
    prompt = new ClientPrompt(promptPath);
    prompt->promptAndWait(5000);
    QVERIFY(prompt->completed());
    QVERIFY(!prompt->dismissed());
    delete prompt;

    // make sure the collection is gone
    QCOMPARE(ifaceCollection.call("Introspect").type(), QDBusMessage::ErrorMessage);

    // make sure the CollectionDeleted signal was sent
    if(deletedSpy.size() < 1) {
        deletedSpy.waitForSignal(5000);
    }
    QCOMPARE(deletedSpy.size(), 1);
    QCOMPARE(deletedSpy.takeFirst(), collectionPath);

    // close the session
    QDBusInterface(DBUS_SERVICE, sessionPath.path()).call("Close");
}

void ServiceTest::blockingItem()
{
    // TODO: test
}

void ServiceTest::cleanupTestCase()
{
    delete m_service;

    QCA::deinit();
}

QTEST_KDEMAIN_CORE(ServiceTest)

ObjectPathSignalSpy::ObjectPathSignalSpy(QDBusAbstractInterface *iface,
        const char *signal)
{
    m_valid = connect(iface, signal, SLOT(slotReceived(QDBusObjectPath)));
}

bool ObjectPathSignalSpy::isValid() const
{
    return m_valid;
}

void ObjectPathSignalSpy::waitForSignal(int time)
{
    waitForSignals(1, time);
}

void ObjectPathSignalSpy::waitForSignals(int num, int time)
{
    m_numWaiting = num;
    if(count() < m_numWaiting) {
        QEventLoop loop;
        loop.connect(this, SIGNAL(doneWaiting()), SLOT(quit()));
        QTimer::singleShot(time, &loop, SLOT(quit()));
        loop.exec();
    }
}

void ObjectPathSignalSpy::slotReceived(const QDBusObjectPath &objectPath)
{
    append(objectPath);
    if(count() >= m_numWaiting) {
        emit doneWaiting();
    }
}

ClientPrompt::ClientPrompt(QDBusObjectPath promptPath)
    : m_completed(false), m_dismissed(false),
      m_interface(DBUS_SERVICE, promptPath.path())
{
    Q_ASSERT(m_interface.isValid());
    connect(&m_interface, SIGNAL(Completed(bool,QDBusVariant)),
            SLOT(slotCompleted(bool,QDBusVariant)));
}

void ClientPrompt::promptAndWait(int time)
{
    QDBusMessage reply = m_interface.call(QDBus::Block, "Prompt", QString(""));
    Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
    justWait(time);
}

void ClientPrompt::dismissAndWait(int time)
{
    QDBusMessage reply = m_interface.call(QDBus::Block, "Dismiss");
    Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
    justWait(time);
}

void ClientPrompt::slotCompleted(bool dismissed, const QDBusVariant &result)
{
    m_completed = true;
    m_dismissed = dismissed;
    m_result = result.variant();
    m_loop.quit();
}

void ClientPrompt::justWait(int time)
{
    if(time > 0) {
        QTimer::singleShot(time, &m_loop, SLOT(quit()));
    }
    m_loop.exec();
}

#include "servicetest.moc"
