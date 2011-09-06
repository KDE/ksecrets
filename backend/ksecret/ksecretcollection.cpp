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

#include "ksecretcollection.h"
#include "ksecretcollectionmanager.h"
#include "ksecretjobs.h"
#include "ksecretfile.h"
#include "../securebuffer.h"
#include "../lib/secrettool.h"

#include <klocalizedstring.h>
#include <ksavefile.h>
#include <kstandarddirs.h>

#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include <kdebug.h>

// this must not be changed or else file compatibility is gone!
#define VERIFIER_LENGTH 64

static const QString genericLoadingErrorMessage()
{
    return i18nc("Error message: Generic error loading the ksecret file",
                 "There was an error reading the ksecret file.");
}

static const QString genericSavingErrorMessage()
{
    return i18nc("Error message: Generic error saving the ksecret file",
                 "There was an error writing the ksecret file.");
}

// create a symmetric encryption key from a user-supplied password using
// PBKDF2.
// Returns a zero-length key if creating the key failed.
static QCA::SymmetricKey createKeyFromPassword(const QCA::SecureArray &password,
        int keyLength)
{
    if(!QCA::isSupported("sha1")) {
        return QCA::SymmetricKey();
    }

    QCA::PBKDF2 deriv("sha1");
    return deriv.makeKey(password, QCA::InitializationVector(), keyLength, 10000);
}

KSecretCollection *KSecretCollection::create(const QString &id, const QCA::SecureArray &password,
        BackendCollectionManager *parent, QString &errorMessage)
{
    KSecretCollection *coll = new KSecretCollection(parent);
    coll->m_id = id;
    coll->m_path = KGlobal::dirs()->saveLocation("ksecret") + '/' + id + ".ksecret";
    coll->m_algoHash = KSecretFile::SHA256;
    coll->m_algoCipher = KSecretFile::AES256;

    // initialize default encryption algorithms
    if(!coll->setupAlgorithms(errorMessage)) {
        delete coll;
        return 0;
    }

    // create a new symmetric key
    // TODO: is minimum() right in all cases?
    coll->m_symmetricKey = new QCA::SymmetricKey(coll->m_cipher->keyLength().minimum());
    QCA::Cipher *keyCipher = coll->m_cipher;
    int keyLength = keyCipher->keyLength().minimum();
    QCA::SymmetricKey keyUnlockKey = createKeyFromPassword(password, keyLength);
    if(keyUnlockKey.isEmpty()) {
        delete coll;
        return 0;
    }

    QCA::InitializationVector iv = QCA::InitializationVector(keyLength);
    EncryptedKey *key = new EncryptedKey;
    key->m_type = KSecretFile::KeyPassword;
    key->m_iv = iv.toByteArray();
    keyCipher->setup(QCA::Encode, keyUnlockKey, iv);
    key->m_key.append(keyCipher->update(*coll->m_symmetricKey).toByteArray());
    key->m_key.append(keyCipher->final().toByteArray());
    coll->m_encryptedSymKeys.append(key);

    // build the verifier
    coll->m_verInitVector = QCA::InitializationVector(coll->m_cipher->blockSize());
    // get random data and compute its hash
    QCA::SecureArray randomData = QCA::Random::randomArray(VERIFIER_LENGTH);
    coll->m_hash->clear();
    coll->m_hash->update(randomData);
    randomData.append(coll->m_hash->final());
    // now encrypt randomData
    coll->m_cipher->setup(QCA::Encode, *coll->m_symmetricKey, coll->m_verInitVector);
    coll->m_verEncryptedRandom = coll->m_cipher->update(randomData);
    coll->m_verEncryptedRandom.append(coll->m_cipher->final());
    
    // write new collection to disk
    if (!coll->serialize(errorMessage)) {
        delete coll;
        return 0;
    }
    coll->m_dirty = false;

    return coll;
}

KSecretCollection::KSecretCollection(BackendCollectionManager *parent)
    : BackendCollection(parent), m_dirty(true), m_hash(0), m_mac(0), 
      m_cipher(0), m_symmetricKey(0)
{
    m_syncTimer.setSingleShot(true);
    connect(&m_syncTimer, SIGNAL(timeout()), SLOT(sync()));
}

KSecretCollection::~KSecretCollection()
{
    // TODO: delete items?
    delete m_hash;
    delete m_cipher;
    delete m_mac;
    delete m_symmetricKey;
    qDeleteAll(m_unknownParts);
    qDeleteAll(m_encryptedSymKeys);
    // TODO: make sure there's nothing else to delete
}

QString KSecretCollection::id() const
{
    return m_id;
}

BackendReturn<QString> KSecretCollection::label() const
{
    return m_label;
}

BackendReturn<void> KSecretCollection::setLabel(const QString &label)
{
    // label can only be set if the collection is unlocked
    if(isLocked()) {
        return BackendErrorIsLocked;
    }

    m_label = label;
    setDirty();
    emit collectionChanged(this);
    return BackendNoError;
}

QDateTime KSecretCollection::created() const
{
    return m_created;
}

QDateTime KSecretCollection::modified() const
{
    return m_modified;
}

bool KSecretCollection::isLocked() const
{
    // a collection is unlocked if m_cipher is initialized
    return (m_symmetricKey == 0);
}

BackendReturn<QList<BackendItem*> > KSecretCollection::items() const
{
    QList<BackendItem*> itemList;
    QHash<QString, KSecretItem*>::const_iterator it = m_items.constBegin();
    QHash<QString, KSecretItem*>::const_iterator end = m_items.constEnd();
    for(; it != end; ++it) {
        itemList.append(it.value());
    }

    return itemList;
}

BackendReturn<QList<BackendItem*> > KSecretCollection::searchItems(
    const QMap<QString, QString> &attributes) const
{
    QList<BackendItem*> itemList;
    // create hashes for each of the attributes queried
    QSet<QByteArray> attributeHashes = KSecretItem::createHashes(attributes, m_hash);
    if(attributeHashes.isEmpty()) {
        return items();
    } else {
        // use the first attribute hash to build an initial list of items matching
        QHash<QByteArray, KSecretItem*>::const_iterator resit =
            m_itemHashes.constFind(*attributeHashes.constBegin());
        QHash<QByteArray, KSecretItem*>::const_iterator resend = m_itemHashes.constEnd();
        // now check which of the items in resit match the remainder of the attributes
        // to do this the actual attributes are used instead of the hashes
        for(; resit != resend; ++resit) {
            if(resit.value()->matches(attributes)) {
                itemList.append(resit.value());
            }
        }
        return itemList;
    }
}

UnlockCollectionJob *KSecretCollection::createUnlockJob(const CollectionUnlockInfo &unlockInfo)
{
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
    // stop any sync timers in progress
    m_syncTimer.stop();
    // remove the ksecret file
    if(!QFile::remove(m_path)) {
        return false;
    }

    // emit signals and actually delete
    emit collectionDeleted(this);
    deleteLater();
    return true;
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
            m_items.insert(item->id(), item);
            // new item, signals need to be wired
            connect(item, SIGNAL(itemDeleted(BackendItem*)), SLOT(slotItemDeleted(BackendItem*)));
            connect(item, SIGNAL(itemChanged(BackendItem*)), SIGNAL(itemChanged(BackendItem*)));
            connect(item, SIGNAL(itemChanged(BackendItem*)), SLOT(startSyncTimer()));
            emit itemCreated(item);
        }
        
        // mark the modification date of the collection.
        m_modified = QDateTime::currentDateTime();
        
        // sync
        setDirty();

        return item;
    }
}

const QString &KSecretCollection::path() const
{
    return m_path;
}

void KSecretCollection::slotItemDeleted(BackendItem *item)
{
    KSecretItem *kitem = qobject_cast<KSecretItem*>(item);
    Q_ASSERT(kitem);

    // remove the item as well as item hashes
    if(m_reverseItemHashes.contains(kitem)) {
        Q_FOREACH(const QByteArray & hash, m_reverseItemHashes.value(kitem)) {
            m_itemHashes.remove(hash);
        }
        m_reverseItemHashes.remove(kitem);
    }
    m_items.remove(kitem->id());
    
    // sync
    setDirty();

    emit itemDeleted(item);
}

void KSecretCollection::changeAttributeHashes(KSecretItem *item)
{
    Q_ASSERT(item);

    // remove previous item hashes
    if(m_reverseItemHashes.contains(item)) {
        QSet<QByteArray> oldHashes = m_reverseItemHashes.value(item);
        Q_FOREACH(const QByteArray & hash, oldHashes) {
            m_itemHashes.remove(hash, item);
        }
    }

    // insert new hashes
    QSet<QByteArray> attributeHashes = item->createAttributeHashes(m_hash);
    Q_FOREACH(const QByteArray & hash, attributeHashes) {
        m_itemHashes.insert(hash, item);
    }
    m_reverseItemHashes.insert(item, attributeHashes);
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

KSecretCollection *KSecretCollection::deserialize(const QString &path,
        KSecretCollectionManager *parent,
        QString &errorMessage)
{
    QFile device(path);
    KSecretFile file(&device, KSecretFile::Read);
    if(!file.isValid()) {
        errorMessage = i18nc("Error message: collection file to be opened does not exist",
                             "Collection does not exist.");
        return 0;
    }

    KSecretCollection *coll = new KSecretCollection(parent);
    coll->m_path = path;

    if(!coll->deserializeHeader(file, errorMessage) ||
            !coll->deserializeParts(file, errorMessage)) {
        delete coll;
        return 0;
    }

    return coll;
}

bool KSecretCollection::tryUnlock()
{
    // TODO: after unlocking, check integrity by verifying the MACs.

    // before actually trying to unlock, check if the key matches by decrypting
    // the verifier.
    if ( m_symmetricKey == 0)
        return false;
    
    m_cipher->setup(QCA::Decode, *m_symmetricKey, m_verInitVector);
    QCA::SecureArray verifier = m_cipher->update(m_verEncryptedRandom);
    verifier.append(m_cipher->final());
    QCA::SecureArray randomData(VERIFIER_LENGTH);
    QCA::SecureArray verifierHash(verifier.size() - VERIFIER_LENGTH);
    int i = 0;
    for(; i < VERIFIER_LENGTH; ++i) {
        randomData[i] = verifier[i];
    }
    for(; i < verifier.size(); ++i) {
        verifierHash[i - VERIFIER_LENGTH] = verifier[i];
    }
    // rebuild the hash and compare to the decrypted one
    m_hash->clear();
    m_hash->update(randomData);
    if(verifierHash != m_hash->final()) {
        // hashes don't match, the master key is wrong.
        return false;
    }

    QCA::SecureArray decryptedPart;
    Q_FOREACH(const QByteArray & partContents, m_encryptedItemParts) {
        if(!deserializePartEncrypted(partContents, decryptedPart)) {
            return false;
        }

        // deserialize the items inside the decrypted part
        SecureBuffer device(&decryptedPart);
        KSecretFile itemFile(&device, KSecretFile::Read);
        if(!deserializeItemsUnlocked(itemFile)) {
            return false;
        }
    }

    emit collectionChanged(this);

    return true;
}

BackendReturn<bool> KSecretCollection::tryUnlockPassword(const QCA::SecureArray &password)
{
    // TODO: reference/open counting?

    bool unlocked = false;

    QCA::Cipher *keyCipher = m_cipher;
    int keyLength = keyCipher->keyLength().minimum();
    QCA::SymmetricKey keyUnlockKey = createKeyFromPassword(password, keyLength);
    // try decrypting any of the symmetric keys using the keyUnlockKey
    Q_FOREACH(EncryptedKey * key, m_encryptedSymKeys) {
        if(key->m_type == KSecretFile::KeyPassword) {
            keyCipher->setup(QCA::Decode, keyUnlockKey, key->m_iv);
            QCA::SecureArray unlockedKey = keyCipher->update(key->m_key);
            unlockedKey.append(keyCipher->final());
            m_symmetricKey = new QCA::SymmetricKey(unlockedKey);

            unlocked = tryUnlock();
            if(unlocked) {
                break;
            }
        }
    }

    return unlocked;
}

BackendReturn<bool> KSecretCollection::lock()
{
    // TODO: reference/open counting?

    // TODO: emit signals?

    if(isLocked()) {
        return true;
    } else {
        if (m_dirty) {
            QString errorMessage;
            if(!serialize(errorMessage)) {
                return BackendReturn<bool>(false, BackendErrorOther, errorMessage);
            }
            m_dirty = false;
        }

        // remove individual item secrets
        QHash<QString, KSecretItem*>::iterator it = m_items.begin();
        QHash<QString, KSecretItem*>::iterator end = m_items.end();
        for(; it != end; ++it) {
            // FIXME: it.value()->removeSecrets();
        }

        // remove symmetric key
        delete m_symmetricKey;
        m_symmetricKey = 0;

        emit collectionChanged(this);

        return true;
    }
}

bool KSecretCollection::setupAlgorithms(QString &errorMessage)
{
    // figure out algorithms to use
    switch(m_algoHash) {

    case KSecretFile::SHA256:
        if(!QCA::isSupported("sha256") || !QCA::isSupported("hmac(sha256)")) {
            errorMessage = i18nc("Error message: unsupported hashing algorithm SHA256 used",
                                 "The hashing algorithm SHA256 is not supported by your installation.");
            return false;
        }
        m_hash = new QCA::Hash("sha256");
        m_mac = new QCA::MessageAuthenticationCode("hmac(sha256)", QCA::SymmetricKey());
        break;

    default:
        errorMessage = i18nc("Error message: unknown hashing algorithm used",
                             "The file uses an unknown hashing algorithm.");
        return false;
    }

    switch(m_algoCipher) {

    case KSecretFile::AES256:
        if(!QCA::isSupported("aes256-cbc-pkcs7")) {
            errorMessage = i18nc("Error message: unsupported encryption algorithm AES256 used",
                                 "The encryption algorithm AES256 is not supported by your installation.");
            return false;
        }
        m_cipher = new QCA::Cipher("aes256", QCA::Cipher::CBC, QCA::Cipher::PKCS7);
        break;

    default:
        errorMessage = i18nc("Error message: unknown encryption algorithm used",
                             "The file uses an unknown encryption algorithm.");
        return false;
    }

    return true;
}

bool KSecretCollection::deserializeHeader(KSecretFile &file, QString &errorMessage)
{
    // magic, version numbers
    quint32 versionMajor;
    quint32 versionMinor;
    if(!file.readMagic() || !file.readUint(&versionMajor) || !file.readUint(&versionMinor)) {
        errorMessage = i18nc("Error message: collection has wrong file format or is corrupted",
                             "Collection is not a ksecret file or is corrupted.");
        return false;
    }

    // check if version number matches something we understand
    if(versionMajor > VERSION_MAJOR) {
        errorMessage = i18nc("Error message: collection's file format is too recent",
                             "The file format used is too recent.");
        return false;
    }

    // algorithms
    if(!file.readUint(&m_algoHash)) {
        errorMessage = genericLoadingErrorMessage();
        return false;
    }

    if (!file.readUint(&m_algoCipher)) {
        errorMessage = genericLoadingErrorMessage();
        return false;
    }

    if(!setupAlgorithms(errorMessage)) {
        // TODO: error message
        return false;
    }

    // verifier
    if(!file.readSecret(&m_verInitVector)) {
        errorMessage = genericLoadingErrorMessage();
        return false;
    }
    
    if(!file.readSecret(&m_verEncryptedRandom)) {
        errorMessage = genericLoadingErrorMessage();
        return false;
    }

    return true;
}

bool KSecretCollection::deserializeParts(KSecretFile &file, QString &errorMessage)
{
    // parts table
    quint32 numParts;
    if(!file.readUint(&numParts)) {
        errorMessage = genericLoadingErrorMessage();
        return false;
    }
    QList<FilePartEntry> filePartEntries;
    quint32 partType;
    quint32 partPos;
    quint32 partLength;
    for(quint32 i = 0; i < numParts; ++i) {
        if(!file.readUint(&partType) || !file.readUint(&partPos) || !file.readUint(&partLength)) {
            errorMessage = genericLoadingErrorMessage();
            return false;
        }

        FilePartEntry part;
        part.m_type = partType;
        part.m_position = partPos;
        part.m_length = partLength;
        filePartEntries.append(part);
    }

    // read parts
    Q_FOREACH(const FilePartEntry & filePartEntry, filePartEntries) {
        // read the contents into a bytearray
        QByteArray contents;
        if(!file.readPart(&contents, filePartEntry.m_position, filePartEntry.m_length)) {
            errorMessage = genericLoadingErrorMessage();
            return false;
        }

        bool rc;
        switch(filePartEntry.m_type) {

        case KSecretFile::PartItemHashes:
            rc = deserializePartItemHashes(contents);
            break;

        case KSecretFile::PartSymKey:
            rc = deserializePartSymKey(contents);
            break;

        case KSecretFile::PartItems:
            m_encryptedItemParts.append(contents);
            rc = true;
            break;

        case KSecretFile::PartAcls:
            rc = deserializePartAcls(contents);
            break;

        case KSecretFile::PartConfig:
            rc = deserializePartConfig(contents);
            break;

        case KSecretFile::PartCollProps:
            rc = deserializePartCollProps(contents);
            break;

        default: // unknown part type
            UnknownFilePart *ufp = new UnknownFilePart;
            ufp->m_type = filePartEntry.m_type;
            ufp->m_contents = contents;
            m_unknownParts.append(ufp);
            rc = true;
            break;
        }

        if(!rc) {
            errorMessage = genericLoadingErrorMessage();
            return false;
        }
    }

    return true;
}

bool KSecretCollection::deserializePartCollProps(const QByteArray &partContents)
{
    QBuffer buffer;
    buffer.setData(partContents);
    KSecretFile file(&buffer, KSecretFile::Read);
    if(!file.isValid()) {
        return false;
    }

    QByteArray partProps;
    QCA::SecureArray propsHash;
    if(!file.readBytearray(&partProps) || 
        !file.readSecret(&propsHash)) {
        return false;
    }
    // TODO: debug this MAC validation sequence
//     m_mac->clear();
//     m_mac->update( QCA::SecureArray( partProps ) );
//     if ( m_mac->final().toByteArray() == propsHash.toByteArray() ) {
//         kDebug() << "MAC failed on collection properties block";
//         return false;
//     }

    QBuffer propsBuffer(&partProps);
    KSecretFile propsFile(&propsBuffer, KSecretFile::Read);
    if(!propsFile.isValid()) {
        return false;
    }

    if( !propsFile.readString(&m_id) || 
        !propsFile.readString(&m_label) ||
        !propsFile.readDatetime(&m_created) || 
        !propsFile.readDatetime(&m_modified)) {
        return false;
    }

    return true;
}

bool KSecretCollection::deserializePartItemHashes(const QByteArray &partContents)
{
    QBuffer buffer;
    buffer.setData(partContents);
    KSecretFile file(&buffer, KSecretFile::Read);
    quint32 numItems;
    if(!file.isValid() || !file.readUint(&numItems)) {
        return false;
    }

    // read each item hash
    KSecretItem *item;
    for(quint32 i = 0; i < numItems; ++i) {
        QString itemId;
        quint32 numAttribs;
        if(!file.readString(&itemId) || !file.readUint(&numAttribs)) {
            return false;
        }

        // read each attribute
        QSet<QByteArray> attributeHashes;
        for(quint32 j = 0; j < numAttribs; ++j) {
            QByteArray attributeHash;
            if(!file.readBytearray(&attributeHash)) {
                return false;
            }
            attributeHashes.insert(attributeHash);
        }

        item = new KSecretItem(itemId, this);
        connect(item, SIGNAL(attributesChanged(KSecretItem*)),
                SLOT(changeAttributeHashes(KSecretItem*)));
        Q_FOREACH(const QByteArray & hash, attributeHashes) {
            m_itemHashes.insert(hash, item);
        }

        // append hashes to the list of reverse hashes
        m_reverseItemHashes.insert(item, attributeHashes);
    }

    return true;
}

bool KSecretCollection::deserializePartSymKey(const QByteArray &partContents)
{
    QBuffer buffer;
    buffer.setData(partContents);
    KSecretFile file(&buffer, KSecretFile::Read);
    quint32 keyType;
    QByteArray keyData;
    if(!file.isValid() || !file.readUint(&keyType) || !file.readBytearray(&keyData)) {
        return false;
    }
    EncryptedKey *key = new EncryptedKey;
    key->m_type = keyType;
    key->m_key = keyData;
    m_encryptedSymKeys.append(key);
    return true;
}

bool KSecretCollection::deserializePartAcls(const QByteArray &partContents)
{
    QBuffer buffer;
    buffer.setData(partContents);
    KSecretFile file(&buffer, KSecretFile::Read);
    if(!file.isValid()) {
        return false;
    }

    QByteArray partAcls;
    if( !file.readBytearray(&partAcls) || 
        !file.readBytearray(&m_aclsMac)) {
        return false;
    }

    QBuffer aclsBuffer(&partAcls);
    KSecretFile aclsFile(&aclsBuffer, KSecretFile::Read);

    quint32 numAcls;
    if( !aclsFile.isValid() || 
        !aclsFile.readUint(&numAcls)) {
        return false;
    }

    QString path;
    quint32 value;
    ApplicationPermission perm;
    for(quint32 i = 0; i < numAcls; ++i) {
        if(!aclsFile.readString(&path) || !file.readUint(&value)) {
            return false;
        }

        switch(value) {

        case PermissionAsk:
            perm = PermissionAsk;
            break;

        case PermissionDeny:
            perm = PermissionDeny;
            break;

        case PermissionAllow:
            perm = PermissionAllow;
            break;

        default:
            // unknown acl rule
            m_unknownAcls.insert(path, value);
            continue;
        }

        m_acls.insert(path, perm);
    }

    if (!aclsFile.readString( &m_creatorApplication ))
        return false;
    
    return true;
}

bool KSecretCollection::deserializePartConfig(const QByteArray &partContents)
{
    QBuffer buffer;
    buffer.setData(partContents);
    KSecretFile file(&buffer, KSecretFile::Read);
    if(!file.isValid()) {
        return false;
    }

    QByteArray partConfig;
    if(!file.readBytearray(&partConfig) || !file.readBytearray(&m_configValuesMac)) {
        return false;
    }

    QBuffer configBuffer(&partConfig);
    KSecretFile configFile(&configBuffer, KSecretFile::Read);
    quint32 numConfigValues;
    if(!configFile.isValid() || !configFile.readUint(&numConfigValues)) {
        return false;
    }

    QString configKey;
    QByteArray configValue;
    for(quint32 i = 0; i < numConfigValues; ++i) {
        if(!configFile.readString(&configKey) || !configFile.readBytearray(&configValue)) {
            return false;
        }

        if(configKey == QLatin1String("CloseScreensaver")) {
            if(configValue.size() == 1 && configValue[0] == '0') {
                m_cfgCloseScreensaver = false;
            }
        } else if(configKey == QLatin1String("CloseIfUnused")) {
            if(configValue.size() == 1 && configValue[0] == '0') {
                m_cfgCloseIfUnused = false;
            }
        } else if(configKey == QLatin1String("CloseUnusedTimeout")) {
            bool ok;
            m_cfgCloseUnusedTimeout = configValue.toUInt(&ok);
            if(!ok) {
                m_cfgCloseUnusedTimeout = 30;
            }
        } else {
            m_cfgUnknownKeys.insert(configKey, configValue);
        }
    }

    return true;
}

bool KSecretCollection::deserializeItemsUnlocked(KSecretFile &file)
{
    Q_ASSERT(file.isValid());

    KSecretItem *item;
    quint32 numItems;
    if(!file.readUint(&numItems)) {
        return false;
    }

    QString itemId;
    for(quint32 i = 0; i < numItems; ++i) {
        // read the identifier of the item
        if(!file.readString(&itemId)) {
            return false;
        }

        // find the item matching the identifier
        bool recreateHashes = false;
        if(!m_items.contains(itemId)) {
            // item without a hash. ignore this inconsistency and simply
            // load the item, creating the hashes on-the-go.
            item = new KSecretItem(itemId, this);
            connect(item, SIGNAL(attributesChanged(KSecretItem*)),
                    SLOT(changeAttributeHashes(KSecretItem*)));
            m_items.insert(itemId, item);
            recreateHashes = true;
        } else {
            item = m_items.value(itemId);
        }

        QCA::SecureArray itemData;
        if(!file.readSecret(&itemData)) {
            return false;
        }

        SecureBuffer device(&itemData);
        KSecretFile itemFile(&device, KSecretFile::Read);
        if(!item->deserializeUnlocked(itemFile)) {
            return false;
        }

        if(recreateHashes) {
            changeAttributeHashes(item);
        }
    }
    return true;
}

bool KSecretCollection::deserializePartEncrypted(const QByteArray &partContents,
        QCA::SecureArray &decryptedPart)
{
    Q_ASSERT(m_symmetricKey);

    QBuffer buffer;
    buffer.setData(partContents);
    KSecretFile file(&buffer, KSecretFile::Read);
    if(!file.isValid()) {
        return false;
    }

    QCA::SecureArray ivdata;
    if(!file.readSecret(&ivdata)) {
        return false;
    }
    QCA::InitializationVector iv(ivdata);

    QCA::SecureArray encryptedPart;
    if(!file.readSecret(&encryptedPart)) {
        return false;
    }

    // decrypt the data
    m_cipher->setup(QCA::Decode, *m_symmetricKey, iv);
    decryptedPart = m_cipher->update(encryptedPart);
    if(!m_cipher->ok()) {
        return false;
    }
    decryptedPart.append(m_cipher->final());
    if(!m_cipher->ok()) {
        return false;
    }

    return true;
}

bool KSecretCollection::serialize(QString &errorMessage) const
{
    KSaveFile device(m_path);
    KSecretFile file(&device, KSecretFile::Write);
    if(!file.isValid()) {
        errorMessage = i18nc("Error message: collection file could not be opened for writing",
                             "Collection file could not be opened for writing");
        kDebug() << errorMessage;
        return false;
    }

    if(!serializeHeader(file)) {
        errorMessage = genericSavingErrorMessage();
        kDebug() << errorMessage;
        return false;
    }

    if(!serializeParts(file)) {
        errorMessage = genericSavingErrorMessage();
        kDebug() << errorMessage;
        return false;
    }
    
    return true;
}

bool KSecretCollection::serializeHeader(KSecretFile &file) const
{
    // write the ksecret file header
    if(!file.writeMagic() || !file.writeUint(VERSION_MAJOR) || !file.writeUint(VERSION_MINOR)) {
        return false;
    }

    // algorithms
    if(!file.writeUint(m_algoHash)) {
        return false;
    }

    if (!file.writeUint(m_algoCipher)) {
        return false;
    }

    if(!file.writeSecret(m_verInitVector)) {
        return false;
    }
    
    if(!file.writeSecret(m_verEncryptedRandom)) {
        return false;
    }

    return true;
}

bool KSecretCollection::serializeParts(KSecretFile &file) const
{
    QString errorMessage;
    // TODO: only save if the collection is open.

    // determine the number of parts
    // properties part + item hash part + items part + acls part + config part + number of keys +
    // number of unknown parts = 5 + number of keys + number of unknown parts
    int curFilePartEntry = 0;
    quint32 numParts = 5 + m_encryptedSymKeys.size() + m_unknownParts.size();
    quint32 partTableSize = 4 + numParts * 12;
    QList<FilePartEntry> filePartEntries;
    for(quint32 i = 0; i < numParts; ++i) {
        filePartEntries.append(FilePartEntry());
    }
    // remember the part table position and skip the part table so it can be written
    // later.
    quint64 partTablePos = file.pos();
    if(!file.seek(partTablePos + partTableSize)) {
        return false;
    }

    // collection properties part
    if(!serializePropertiesPart(file, filePartEntries[curFilePartEntry])) {
        return false;
    }
    curFilePartEntry++;

    // config part
    if(!serializeConfigPart(file, filePartEntries[curFilePartEntry])) {
        return false;
    }
    curFilePartEntry++;

    // acls part
    filePartEntries[curFilePartEntry].m_type = KSecretFile::PartAcls;
    filePartEntries[curFilePartEntry].m_position = (quint32)file.pos();
    QBuffer buffer;
    KSecretFile device(&buffer, KSecretFile::Write);
    QHash<QString, ApplicationPermission>::const_iterator it = m_acls.constBegin();
    QHash<QString, ApplicationPermission>::const_iterator end = m_acls.constEnd();
    for(; it != end; ++it) {
        if(!file.writeString(it.key()) || !file.writeUint((quint32)it.value())) {
            return false;
        }
    }
    if(!serializeAuthenticated(buffer.data(), file)) {
        return false;
    }
    filePartEntries[curFilePartEntry].m_length =
        (quint32)file.pos() - filePartEntries[curFilePartEntry].m_position;
    curFilePartEntry++;

    // item hash part
    if(!serializeItemHashes(file, filePartEntries[curFilePartEntry])) {
        return false;
    }
    curFilePartEntry++;

    // items part
    if(!serializeItems(file, filePartEntries[curFilePartEntry])) {
        return false;
    }
    curFilePartEntry++;

    // keys
    Q_FOREACH(EncryptedKey * key, m_encryptedSymKeys) {
        filePartEntries[curFilePartEntry].m_type = KSecretFile::PartSymKey;
        filePartEntries[curFilePartEntry].m_position = (quint32)file.pos();
        if(!file.writeUint(key->m_type) || !file.writeBytearray(key->m_key) ||
                !file.writeBytearray(key->m_iv)) {
            return false;
        }
        filePartEntries[curFilePartEntry].m_length =
            (quint32)file.pos() - filePartEntries[curFilePartEntry].m_position;
        curFilePartEntry++;
    }

    // unknown parts
    Q_FOREACH(UnknownFilePart * unknown, m_unknownParts) {
        filePartEntries[curFilePartEntry].m_type = unknown->m_type;
        filePartEntries[curFilePartEntry].m_position = (quint32)file.pos();
        if(!file.writeBytearray(unknown->m_contents)) {
            return false;
        }
        filePartEntries[curFilePartEntry].m_length =
            (quint32)file.pos() - filePartEntries[curFilePartEntry].m_position;
        curFilePartEntry++;
    }

    // write part table
    if(!file.seek(partTablePos) || !file.writeUint(numParts)) {
        return false;
    }

    Q_FOREACH(const FilePartEntry & part, filePartEntries) {
        if(!file.writeUint(part.m_type) || !file.writeUint(part.m_position) ||
                !file.writeUint(part.m_length)) {
            return false;
        }
    }

    file.close();
    return true;
}

bool KSecretCollection::serializeAclsPart(KSecretFile &file, FilePartEntry &entry) const
{
    entry.m_type = KSecretFile::PartAcls;
    entry.m_position = (quint32)file.pos();

    // build temporary buffer containing acls
    QBuffer aclsBuffer;
    KSecretFile tempFile(&aclsBuffer, KSecretFile::Write);
    if(!tempFile.writeUint(m_acls.size())) {
        return false;
    }

    QHash<QString, ApplicationPermission>::const_iterator it = m_acls.constBegin();
    QHash<QString, ApplicationPermission>::const_iterator end = m_acls.constEnd();
    for(; it != end; ++it) {
        if( !tempFile.writeString(it.key()) || 
            !tempFile.writeUint((quint32)it.value())) {
            return false;
        }
    }

    QMap<QString, int>::const_iterator it2 = m_unknownAcls.constBegin();
    QMap<QString, int>::const_iterator end2 = m_unknownAcls.constEnd();
    for(; it2 != end2; ++it2) {
        if( !tempFile.writeString(it2.key()) || 
            !tempFile.writeUint(it2.value())) {
            return false;
        }
    }

    if (!tempFile.writeString( m_creatorApplication ))
        return false;
    
    if(!serializeAuthenticated(aclsBuffer.data(), file)) {
        return false;
    }
    entry.m_length = (quint32)file.pos() - entry.m_position;
    return true;
}

bool KSecretCollection::serializePropertiesPart(KSecretFile &file, FilePartEntry &entry) const
{
    entry.m_type = KSecretFile::PartCollProps;
    entry.m_position = (quint32)file.pos();

    // build temporary buffer containing properties
    QBuffer propsBuffer;
    KSecretFile tempFile(&propsBuffer, KSecretFile::Write);

    if( !tempFile.writeString(m_id) || 
        !tempFile.writeString(m_label) ||
        !tempFile.writeDatetime(m_created) || 
        !tempFile.writeDatetime(m_modified)) {
        return false;
    }

    tempFile.close();
    if(!serializeAuthenticated(propsBuffer.data(), file)) {
        return false;
    }
    entry.m_length = (quint32)file.pos() - entry.m_position;

    return true;
}

bool KSecretCollection::serializeConfigPart(KSecretFile &file, FilePartEntry &entry) const
{
    entry.m_type = KSecretFile::PartConfig;
    entry.m_position = (quint32)file.pos();

    // build temporary buffer containing config keys and values
    QBuffer configBuffer;
    KSecretFile tempFile(&configBuffer, KSecretFile::Write);
    if(!tempFile.writeUint(3 + m_cfgUnknownKeys.size())) {
        return false;
    }

    // save known configuration values
    QByteArray valCloseScreensaver;
    valCloseScreensaver.setNum((int)m_cfgCloseScreensaver);
    QByteArray valCloseIfUnused;
    valCloseIfUnused.setNum((int)m_cfgCloseIfUnused);
    QByteArray valCloseUnusedTimeout;
    valCloseUnusedTimeout.setNum(m_cfgCloseUnusedTimeout);
    if(!tempFile.writeString("CloseScreensaver") ||
            !tempFile.writeBytearray(valCloseScreensaver) ||
            !tempFile.writeString("CloseIfUnused") ||
            !tempFile.writeBytearray(valCloseIfUnused) ||
            !tempFile.writeString("CloseUnusedTimeout") ||
            !tempFile.writeBytearray(valCloseUnusedTimeout)) {
        return false;
    }

    // save unknown configuration values
    QMap<QString, QByteArray>::const_iterator it = m_cfgUnknownKeys.constBegin();
    QMap<QString, QByteArray>::const_iterator end = m_cfgUnknownKeys.constEnd();
    for(; it != end; ++it) {
        if(!tempFile.writeString(it.key()) || !tempFile.writeBytearray(it.value())) {
            return false;
        }
    }

    tempFile.close();
    if(!serializeAuthenticated(configBuffer.data(), file)) {
        return false;
    }
    entry.m_length = (quint32)file.pos() - entry.m_position;

    return true;
}

bool KSecretCollection::serializeItemHashes(KSecretFile &file, FilePartEntry &entry) const
{
    entry.m_type = KSecretFile::PartItemHashes;
    entry.m_position = (quint32)file.pos();

    // as the hashes are stored as hash-value => item and they have to be written
    // to the file the other way round (item => hash-value, hash-value, ...), we
    // have to manually reverse them.

    QHash<KSecretItem*, QList<QByteArray> > hashes;

    QMultiHash<QByteArray, KSecretItem*>::const_iterator it = m_itemHashes.constBegin();
    const QMultiHash<QByteArray, KSecretItem*>::const_iterator end = m_itemHashes.constEnd();
    for(; it != end; ++it) {
        hashes[it.value()].append(it.key());
    }

    if(!file.writeUint(hashes.size())) {
        return false;
    }

    QHash<KSecretItem*, QList<QByteArray> >::const_iterator it2 = hashes.constBegin();
    QHash<KSecretItem*, QList<QByteArray> >::const_iterator end2 = hashes.constEnd();
    for(; it2 != end2; ++it2) {
        if(!file.writeString(it2.key()->id()) || !file.writeUint(it2.value().size())) {
            return false;
        }
        QList<QByteArray> hv = it2.value();
        Q_FOREACH(const QByteArray & hash, hv) {
            if(!file.writeBytearray(hash)) {
                return false;
            }
        }
    }

    entry.m_length = (quint32)file.pos() - entry.m_position;

    return true;
}

bool KSecretCollection::serializeItems(KSecretFile &file, FilePartEntry &entry) const
{
    Q_ASSERT(file.isValid());

    entry.m_type = KSecretFile::PartItems;
    entry.m_position = (quint32)file.pos();

    // construct a file to write the unlocked items to
    SecureBuffer device;
    KSecretFile tempFile(&device, KSecretFile::Write);

    if(!tempFile.writeUint(m_items.size())) {
        return false;
    }

    QHash<QString, KSecretItem*>::const_iterator it = m_items.constBegin();
    const QHash<QString, KSecretItem*>::const_iterator end = m_items.constEnd();
    for(; it != end; ++it) {
        if(!it.value()->serializeUnlocked(tempFile)) {
            return false;
        }
    }

    bool rc = serializeEncrypted(device.buffer(), file);
    if(rc) {
        entry.m_length = (quint32)file.pos() - entry.m_position;
        return true;
    } else {
        return false;
    }
}

bool KSecretCollection::serializeEncrypted(const QCA::SecureArray &data, KSecretFile &file) const
{
    Q_ASSERT(file.isValid());
    // TODO: those errors either have to be caught before calling this method
    //       or inside.
    Q_ASSERT(m_symmetricKey);
    Q_ASSERT(m_hash);
    Q_ASSERT(m_cipher);

    QCA::InitializationVector iv(m_cipher->blockSize());
    if(!file.writeSecret(iv)) {
        return false;
    }

    // TODO: random padding?

    // encrypt the data
    m_cipher->setup(QCA::Encode, *m_symmetricKey, iv);
    QCA::SecureArray encryptedPart = m_cipher->update(data);
    if(!m_cipher->ok()) {
        return false;
    }
    encryptedPart.append(m_cipher->final());
    if(!m_cipher->ok()) {
        return false;
    }

    // write the encrypted data to file
    if(!file.writeSecret(encryptedPart)) {
        return false;
    }

    return true;
}

bool KSecretCollection::serializeAuthenticated(const QByteArray &data, KSecretFile &file) const
{
    Q_ASSERT(m_mac);

    // write actual data first
    if(!file.writeBytearray(data)) {
        return false;
    }

    // compute the HMAC for the data
    m_mac->clear();
    m_mac->update(QCA::SecureArray(data));
    QCA::SecureArray dataHash = m_mac->final();

    return file.writeSecret(dataHash);
}

ApplicationPermission KSecretCollection::applicationPermission(const QString& path) const
{
    ApplicationPermission result = PermissionUndefined;
    if ( m_acls.contains(  path ) )
        result = m_acls[path];
    return result;
}

bool KSecretCollection::setApplicationPermission(const QString& path, ApplicationPermission perm)
{
    bool result = false;
    Q_ASSERT( !isLocked() );
    if ( !isLocked() ) {
        if ( !path.isEmpty() && QFile::exists( path ) ) {
            m_acls[ path ] = perm;
            
            // sync
            setDirty();
            
            result = true;
        }
    }
    return result;
}

bool KSecretCollection::setCreatorApplication(QString exePath)
{
    if ( m_creatorApplication.isEmpty() || exePath.isEmpty() )
        return false;
    m_creatorApplication = exePath;
    return true;
}

void KSecretCollection::setDirty(bool dirty)
{
    if ( dirty != m_dirty ) {
        // no change is possible if the collection is locked
        Q_ASSERT( !isLocked() );
        if ( !isLocked() ) {
            m_dirty = dirty;
            if ( dirty ) {
                startSyncTimer();
            }
        }
    }
}


#include "ksecretcollection.moc"
