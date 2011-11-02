/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
 * Copyright 2011, Valentin Rusu <kde@rusu.info>
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

#include "ksecretcollection.h"
#include "ksecretcollectionmanager.h"
#include "ksecretjobs.h"
#include "../securebuffer.h"
#include "../lib/secrettool.h"
#include "ksecretencryptionfilter.h"
#include "ksecretdevice.h"
#include "ksecretstream.h"

#include <klocalizedstring.h>
#include <ksavefile.h>
#include <kstandarddirs.h>

#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include <kdebug.h>
#include <QFileInfo>

static QCA::SecureArray replaceWithDefaultIfEmpty( const QCA::SecureArray &password ) 
{
    if ( password.isEmpty() ) {
        return QCA::SecureArray( "ksecretdefaultpwd" );
    }
    else {
        return password;
    }
}

KSecretCollection *KSecretCollection::create(const QString &id, const QCA::SecureArray &password,
        BackendCollectionManager *parent, QString &errorMessage)
{
    KSecretCollection *coll = new KSecretCollection(parent);
    coll->m_path = KGlobal::dirs()->saveLocation("ksecret") + '/' + id + ".ksecret";
    coll->m_encryptionFilter = new KSecretEncryptionFilter( replaceWithDefaultIfEmpty( password ) );
    
    coll->m_pub.m_id = id;

    // write new collection to disk to get a final validation
    coll->m_dirty = true;
    if ( !coll->serialize(errorMessage) ) {
        delete coll;
        return 0;
    }

    return coll;
}

KSecretCollection::KSecretCollection(BackendCollectionManager *parent)
    : BackendCollection(parent), m_locked(), m_encryptionFilter(0), m_dirty(false)
{
    m_syncTimer.setSingleShot(true);
    connect(&m_syncTimer, SIGNAL(timeout()), SLOT(sync()));
}

KSecretCollection::~KSecretCollection()
{
    delete m_encryptionFilter;
}

QString KSecretCollection::id() const
{
    return m_pub.m_id;
}

BackendReturn<QString> KSecretCollection::label() const
{
    return BackendReturn<QString>( m_pub.m_label );
}

BackendReturn<void> KSecretCollection::setLabel(const QString &label)
{
    // label can only be set if the collection is unlocked
    if ( isLocked() ) {
        return BackendErrorIsLocked;
    }
    
    m_pub.m_label = label;
    setDirty();
    emit collectionChanged(this);
    return BackendReturn<void>( BackendNoError );
}

QDateTime KSecretCollection::created() const
{
    return m_secret.m_created;
}

QDateTime KSecretCollection::modified() const
{
    return m_secret.m_modified;
}

bool KSecretCollection::isLocked() const
{
    return m_locked;
}

BackendReturn<QList<BackendItem*> > KSecretCollection::items() const
{
    BackendReturn< QList<BackendItem*> > result;
    if ( isLocked() ) {
        result.setError( BackendErrorIsLocked, i18n("The backend is locked!") );
    }
    else {
        QList<BackendItem*> itemList;
        QHash<QString, KSecretItem*>::const_iterator it = m_secret.m_items.constBegin();
        QHash<QString, KSecretItem*>::const_iterator end = m_secret.m_items.constEnd();
        for(; it != end; ++it) {
            itemList.append(it.value());
        }
        result = itemList;
    }
    
    return result;
}

BackendReturn<QList<BackendItem*> > KSecretCollection::searchItems(
    const QMap<QString, QString> &attributes) const
{
    BackendReturn<QList<BackendItem*> > result;
    if ( isLocked() ) {
        result.setError( BackendErrorIsLocked, i18n("The backend is locked!") );
    }
    else {
        QList<BackendItem*> itemList;
        
        QMap<QString, QString>::const_iterator a = attributes.constBegin();
        for ( ; a != attributes.constEnd() ; a++ ) {
            QString attrKey = a.key();
            QMultiHash< QString, KSecretItem* >::const_iterator it = m_secret.m_lookupHashes.constFind( attrKey );
            for ( ; it != m_secret.m_lookupHashes.constEnd() ; it++ ) {
                BackendItem *item = it.value();
                if ( ! itemList.contains( item ) ) {
                    bool match = false;
                    QString searchAttr = a.value();
                    if ( searchAttr.isEmpty() ) {
                        match = true;
                    }
                    else {
                        QMap< QString, QString > itemAttrs = item->attributes().value();
                        
                        if ( searchAttr.startsWith( QLatin1String("regexp:"), Qt::CaseInsensitive ) ) {
                            QString expr = searchAttr.mid(7);
                            if ( !expr.isEmpty() ) {
                                QRegExp rx( expr );
                                match = rx.exactMatch( expr );
                            }
                        }
                        else {
                            // do exact match
                            if ( itemAttrs.value( attrKey ) == searchAttr ) {
                                match = true;
                            }
                        }
                    }

                    if ( match ) {
                        itemList.append( item );
                    }
                }
            }
        }
        
//         // create hashes for each of the attributes queried
//         Q_ASSERT( m_encryptionFilter != 0 );
//         QList<BackendItem*> itemList;
//         QSet<QByteArray> attributeHashes = m_encryptionFilter->createHashes( attributes);
//         if(attributeHashes.isEmpty()) {
//             itemList = items().value();
//         } else {
//             // use the first attribute hash to build an initial list of items matching
//             QHash<QByteArray, KSecretItem*>::const_iterator resit =
//                 m_secret.m_itemHashes.constFind(*attributeHashes.constBegin());
//             QHash<QByteArray, KSecretItem*>::const_iterator resend = m_secret.m_itemHashes.constEnd();
//             // now check which of the items in resit match the remainder of the attributes
//             // to do this the actual attributes are used instead of the hashes
//             for(; resit != resend; ++resit) {
//                 if(resit.value()->matches(attributes)) {
//                     itemList.append(resit.value());
//                 }
//             }
//         }
        result = itemList;
    }
    return result;
}

UnlockCollectionJob *KSecretCollection::createUnlockJob(const CollectionUnlockInfo &unlockInfo)
{
    unlockInfo.m_collection = this;
    UnlockCollectionJob *job = new KSecretUnlockCollectionJob(unlockInfo, this);
    return job;
}

LockCollectionJob *KSecretCollection::createLockJob()
{
    return new KSecretLockCollectionJob(this);
}

DeleteCollectionJob *KSecretCollection::createDeleteJob(const CollectionDeleteInfo& deleteJobInfo)
{
    deleteJobInfo.m_collection = this;
    return new KSecretDeleteCollectionJob(deleteJobInfo);
}

BackendReturn<bool> KSecretCollection::deleteCollection()
{
    BackendReturn< bool > result;
    // stop any sync timers in progress
    m_syncTimer.stop();
    // remove the ksecret file
    if(!QFile::remove(m_path)) {
        result = false;
        // FIXME: should we also call a result.setError here?
    }

    // emit signals and actually delete
    emit collectionDeleted(this);
    deleteLater();
    
    return result;
}

CreateItemJob *KSecretCollection::createCreateItemJob(const ItemCreateInfo& createInfo)
{
    return new KSecretCreateItemJob(createInfo, this);
}

ChangeAuthenticationCollectionJob *KSecretCollection::createChangeAuthenticationJob( const Peer& peer )
{
    return new KSecretChangeAuthenticationCollectionJob(this, peer);
}

BackendReturn<BackendItem*> KSecretCollection::createItem(const QString &label,
        const QMap<QString, QString> &attributes,
        const QCA::SecureArray &secret,
        const QString& contentType,
        bool replace, bool locked)
{
    // TODO: use locked argument
    Q_UNUSED(locked);

    // only works if unlocked
    if(isLocked()) {
        return BackendReturn<BackendItem*>(0, BackendErrorIsLocked);
    } else {
        KSecretItem *item = 0;
        bool replacing = false;

        // check for duplicates
        BackendReturn<QList<BackendItem*> > foundItems = searchItems(attributes);
        if(!foundItems.isError() && foundItems.value().size() > 0) {
            QList<BackendItem*> oldlist = foundItems.value();
            Q_FOREACH(BackendItem * olditem, oldlist) {
                if(olditem->attributes().value() == attributes) {
                    if(replace) {
                        // replace an existing item
                        item = qobject_cast<KSecretItem*>(olditem);
                        replacing = true;
                    } else {
                        // item existing but should not be replaced
                        return BackendReturn<BackendItem*>(0, BackendErrorAlreadyExists);
                    }
                    break;
                }
            }
        }

        if(!item) {
            item = new KSecretItem(createId(), this);
            connect(item, SIGNAL(attributesChanged(KSecretItem*)),
                    SLOT(changeAttributeHashes(KSecretItem*)));
        }
        item->m_label = label;
        item->m_attributes = attributes;
        item->m_secret = secret;
        item->m_contentType = contentType;

        // insert new item's hashes
        changeAttributeHashes(item);

        if(replacing) {
            emit itemChanged(item);
        } else {
            m_secret.m_items.insert(item->id(), item);
            // new item, signals need to be wired
            connect(item, SIGNAL(itemDeleted(BackendItem*)), SLOT(slotItemDeleted(BackendItem*)));
            connect(item, SIGNAL(itemChanged(BackendItem*)), SLOT(slotItemChanged(BackendItem*)));
            emit itemCreated(item);
        }
        
        // mark the modification date of the collection.
        m_secret.m_modified = QDateTime::currentDateTime();
        
        // sync
        setDirty();

        return BackendReturn<BackendItem*>( item );
    }
}

const QString &KSecretCollection::path() const
{
    return m_path;
}

void KSecretCollection::slotItemChanged(BackendItem* item)
{
    emit itemChanged( item );
    setDirty();
}

void KSecretCollection::slotItemDeleted(BackendItem *item)
{
    KSecretItem *kitem = qobject_cast<KSecretItem*>(item);
    Q_ASSERT(kitem);

    QMultiHash< QString, KSecretItem* >::iterator it = m_secret.m_lookupHashes.begin();
    while ( it != m_secret.m_lookupHashes.end() ) {
        if ( it.value() == item ) {
            it = m_secret.m_lookupHashes.erase( it );
        }
        else {
            ++it;
        }
    }
    
//     // remove the item as well as item hashes
//     if(m_secret.m_reverseItemHashes.contains(kitem)) {
//         Q_FOREACH(const QByteArray & hash, m_secret.m_reverseItemHashes.value(kitem)) {
//             m_secret.m_itemHashes.remove(hash);
//         }
//         m_secret.m_reverseItemHashes.remove(kitem);
//     }
    m_secret.m_items.remove(kitem->id());
    
    // sync
    setDirty();

    emit itemDeleted(item);
}

void KSecretCollection::changeAttributeHashes(KSecretItem *item)
{
    Q_ASSERT(item);

    QMap< QString, QString> attrs = item->attributes().value();
    QMapIterator< QString, QString > it( attrs );
    while ( it.hasNext() ) {
        it.next();
        m_secret.m_lookupHashes.insert( it.key(), item );
    }
    
//     // remove previous item hashes
//     if(m_secret.m_reverseItemHashes.contains(item)) {
//         QSet<QByteArray> oldHashes = m_secret.m_reverseItemHashes.value(item);
//         Q_FOREACH(const QByteArray & hash, oldHashes) {
//             m_secret.m_itemHashes.remove(hash, item);
//         }
//     }
// 
//     // insert new hashes
//     QSet<QByteArray> attributeHashes = m_encryptionFilter->createHashes( item->attributes().value() );
//     Q_FOREACH(const QByteArray & hash, attributeHashes) {
//         m_secret.m_itemHashes.insert(hash, item);
//     }
//     m_secret.m_reverseItemHashes.insert(item, attributeHashes);
}

void KSecretCollection::startSyncTimer()
{
    // mark collection dirty and start sync in 3 seconds.
    // make sure it's really synced to constant changes to the collection don't
    // leave it unsaved for too long.
    if (!m_syncTimer.isActive()) {
        m_syncTimer.start(3000);
    }
}

void KSecretCollection::sync()
{
    if (m_dirty) {
        QString errorMessage;
        if (!serialize(errorMessage)) {
            // try to resync if syncing failed
            startSyncTimer();
        } else {
            m_dirty = false;
        }
    }
}

KSecretCollection* KSecretCollection::createFromFile(const QString& path, BackendCollectionManager* manager, QString& errorMessage)
{
    KSecretCollection *coll = new KSecretCollection( manager );
    coll->m_path = path;

    // NOTE: when deserializing on startup (for exeample) we don't yet havec the password
    // so we'll give an empty one. The first unlock attempt would fail so the unlock job
    // will prompt the user for the password
    coll->m_encryptionFilter = new KSecretEncryptionFilter( replaceWithDefaultIfEmpty("") );
    
    KSecretDevice< QFile > device( path, coll->m_encryptionFilter );
    if ( !device.open( QIODevice::ReadOnly ) ) {
        errorMessage = i18nc("Error message: collection file to be opened does not exist or is not a KSecretsService file",
                             "Collection does not exist.");
        return 0;
    }
    
    KSecretStream stream( &device );
    if ( !stream.isValid() ) {
        errorMessage = i18nc("Error message: collection file to be opened is corrupted",
                             "Collection does not exist.");
        return 0;
    }
    
    stream >> coll->m_pub;
    
    // the rest of the file will be deserialized upon collection unlock as we need the password for that
    coll->m_locked = true;
    return coll;
}

BackendReturn<bool> KSecretCollection::tryUnlock()
{
    if ( !isLocked() ) {
        // already unlocked
        return BackendReturn<bool>( true );
    }
    
    KSecretDevice< QFile > device( m_path, m_encryptionFilter );
    if ( !device.open( QIODevice::ReadOnly ) ) {
        return BackendReturn<bool>( false );
    }
    KSecretStream stream( &device );

    stream >> m_pub;
    device.startEncrypting();
    stream >> m_secret;
    
    if ( !stream.isValid() ) {
        kDebug() << "collection serialization failed";
        return BackendReturn<bool>( false );
    }
    
    // collection is now correctly loaded and unlocked
    m_locked = false;

    foreach( KSecretItem *item, m_secret.m_items ) {
        item->setCollection( this );
        connect(item, SIGNAL(attributesChanged(KSecretItem*)),
                SLOT(changeAttributeHashes(KSecretItem*)));
        
        emit itemChanged( item ); // that is, item was unlocked

        QMapIterator< QString, QString> it( item->attributes().value() );
        while ( it.hasNext() ) {
            it.next();
            m_secret.m_lookupHashes.insert( it.key(), item );
        }
//         QSet<QByteArray> attributeHashes = m_encryptionFilter->createHashes( item->attributes().value() );
//         Q_FOREACH(const QByteArray & hash, attributeHashes) {
//             m_secret.m_itemHashes.insert(hash, item);
//         }
//         m_secret.m_reverseItemHashes.insert(item, attributeHashes);
    }
    
    
    emit collectionChanged(this);

    return BackendReturn<bool>( true );
}

BackendReturn<bool> KSecretCollection::tryUnlockPassword(const QCA::SecureArray &password)
{
    // TODO: reference/open counting?

    return BackendReturn<bool> (
        m_encryptionFilter->setPassword( replaceWithDefaultIfEmpty( password ) ) && 
        tryUnlock().value() );
}

BackendReturn<bool> KSecretCollection::lock()
{
    // TODO: reference/open counting?

    // TODO: emit signals?

    if(isLocked()) {
        return BackendReturn<bool>( true );
    } else {
        if (m_dirty) {
            QString errorMessage;
            if(!serialize(errorMessage)) {
                return BackendReturn<bool>(false, BackendErrorOther, errorMessage);
            }
            m_dirty = false;
        }

        // remove individual item secrets
        qDeleteAll( m_secret.m_items );
        m_secret.m_items.empty();
        
        m_locked = true;
        emit collectionChanged(this);

        return BackendReturn<bool>( true );
    }
}

ApplicationPermission KSecretCollection::applicationPermission(const QString& path) const
{
    ApplicationPermission result = PermissionUndefined;
    if ( !isLocked() ) {
        if ( m_secret.m_acls.contains(  path ) )
            result = m_secret.m_acls[path];
    }
    return result;
}

bool KSecretCollection::setApplicationPermission(const QString& path, ApplicationPermission perm)
{
    bool result = false;
//     Q_ASSERT( !isLocked() );
    if ( !isLocked() ) {
        if ( !path.isEmpty() && QFile::exists( path ) ) {
            m_secret.m_acls[ path ] = perm;
            
            // sync
            setDirty();
            
            result = true;
        }
    }
    return result;
}

bool KSecretCollection::setCreatorApplication(QString exePath)
{
    bool result = false;
    if ( !isLocked() ) {
        if ( !m_secret.m_creatorApplication.isEmpty() && !exePath.isEmpty() ) {
            m_secret.m_creatorApplication = exePath;
            result = true;
        }
    }
    return result;
}

void KSecretCollection::setDirty(bool dirty)
{
    if ( dirty != m_dirty ) {
        // no change is possible if the collection is locked
        Q_ASSERT( !isLocked() );
        if ( !isLocked() ) {
            m_dirty = dirty;
            if ( dirty ) {
                m_secret.m_modified = QDateTime::currentDateTimeUtc();
            }
        }
    }
    startSyncTimer();
}

bool KSecretCollection::serialize(QString &errorMessage) const
{
    if ( !m_dirty )
        return true;
    
    Q_ASSERT( m_encryptionFilter != 0 );

    KSecretDevice< KSaveFile > device( m_path, m_encryptionFilter );
    if ( !device.open( QIODevice::ReadWrite ) ) {
        errorMessage = i18nc("Error message: collection file could not be created",
                             "The disk may be full");
        return false;
    }
    
    KSecretStream ostream( &device );
    ostream << m_pub;
    device.startEncrypting();
    ostream << m_secret;
    
    if ( !ostream.isValid() ) {
        errorMessage = i18nc("Error message: collection contents could not be written to disk",
                             "The disk may be full");
        return false;
    }
    
    if ( !device.finalize() ) {
        errorMessage = i18nc("Error message: collection contents could not be written to disk",
                             "The disk may be full");
        return false;
    }
    
    // ok, the collection was correctly serialized
    m_dirty = false;
    return true;
}

KSecretCollection::SecretData::SecretData() :
    m_cfgCloseScreensaver(false),
    m_cfgCloseIfUnused(false),
    m_cfgCloseUnusedTimeout(0)
{
    m_created = QDateTime::currentDateTimeUtc();
    m_modified = m_created;
}

KSecretCollection::SecretData::~SecretData()
{
    qDeleteAll( m_items );
}

KSecretStream& operator<<(KSecretStream& stream, const KSecretCollection::PublicData& d)
{
    stream << d.m_id;
    stream << d.m_label;
    return stream;
}

KSecretStream& operator>>(KSecretStream& stream, KSecretCollection::PublicData& d)
{
    stream >> d.m_id;
    stream >> d.m_label;
    return stream;
}


KSecretStream& operator << ( KSecretStream& stream, const KSecretCollection::SecretData &d )
{
    stream << d.m_created;
    stream << d.m_modified;
    stream << d.m_cfgCloseScreensaver;
    stream << d.m_cfgCloseIfUnused;
    (QDataStream&)stream << d.m_cfgCloseUnusedTimeout;
    stream << d.m_acls;
    stream << d.m_creatorApplication;
    stream << d.m_items;
    // FIXME: should we also serialize hashes? My opinion is "No"
    return stream;
}

KSecretStream& operator >> ( KSecretStream &stream, KSecretCollection::SecretData &d )
{
    stream >> d.m_created;
    stream >> d.m_modified;
    stream >> d.m_cfgCloseScreensaver;
    stream >> d.m_cfgCloseIfUnused;
    (QDataStream&)stream >> d.m_cfgCloseUnusedTimeout;
    stream >> d.m_acls;
    stream >> d.m_creatorApplication;
    stream >> d.m_items;
    return stream;
}


#include "ksecretcollection.moc"
