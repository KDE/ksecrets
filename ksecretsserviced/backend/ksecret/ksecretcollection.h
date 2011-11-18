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

#ifndef KSECRETCOLLECTION_H
#define KSECRETCOLLECTION_H

#include "../backendcollection.h"
#include "ksecretitem.h"

#include <QtCore/QTimer>
#include "ksecretstream.h"

class KSecretEncryptionFilter;
class KSecretCollectionManager;
class KSecretUnlockCollectionJob;
class KSecretLockCollectionJob;
class KSecretDeleteCollectionJob;
class KSecretCreateItemJob;

/**
 * Represents a part of the ksecret file stored in memory. The part's type
 * is stored as unsigned integer as the part is unknown to this version of
 * ksecretsservice.
 */
struct UnknownFilePart {
    quint32 m_type;
    QByteArray m_contents;
};

/**
 * Holds a description of a ksecret file part.
 */
struct FilePartEntry {
    quint32 m_type;
    quint32 m_position;
    quint32 m_length;
};

/**
 * A collection stored on disk using the ksecret file format.
 */
class KSecretCollection : public BackendCollection
{
    Q_OBJECT

private:
    /**
     * Constructor for loading an existing collection from a file or creating
     * a new collection using create().
     *
     * @param parent the collection manager that loads this collection
     */
    KSecretCollection(BackendCollectionManager *parent);

public:
    /**
     * Create a new collection.
     *
     * @param id id for the new collection
     * @param password password to use for encrypting the collection
     * @param parent parent collection manager
     * @param errorMessage set in case of an error
     * @return the new collection or 0 in case of an error
     */
    static KSecretCollection *create(const QString &id, const QCA::SecureArray &password,
                                     BackendCollectionManager *parent, QString &errorMessage);

    /**
     * This method inspects the given file and creates a collection if it
     * seems containing a valid collection. Note that the contents of the
     * file is not thouroughly checked. This will be done when unlocking
     * the collection is requested
     * 
     */
    static KSecretCollection *createFromFile( const QString& path, BackendCollectionManager *manager, QString &errorMessage );

    /**
     * Destructor
     */
    virtual ~KSecretCollection();

    /**
     * The unique identifier for this collection
     */
    virtual QString id() const;

    /**
     * The human-readable label for this collection.
     * @todo error
     */
    virtual BackendReturn<QString> label() const;

    /**
     * Set this collection's label human-readable label.
     *
     * @todo error
     * @param label the new label for this collection
     */
    virtual BackendReturn<void> setLabel(const QString &label);

    /**
     * The time this collection was created.
     */
    virtual QDateTime created() const;

    /**
     * The time this collection was last modified.
     */
    virtual QDateTime modified() const;

    /**
     * Check whether this collection is locked.
     *
     * @return true if the collection is locked, false if the collection
     *         is unlocked.
     */
    virtual bool isLocked() const;

    /**
     * List all items inside this backend.
     *
     * @return a list containing all items inside this backend. An empty list
     *         either means that no items were found or that an error occurred
     *         (eg. collection needs unlocking before listing the items).
     * @todo error
     */
    virtual BackendReturn<QList<BackendItem*> > items() const;

    /**
     * Return all items whose attributes match the search terms.
     *
     * @param attributes attributes against which the items should be matched. 
     *      If no attributes are given, all items are returned
     * @return a list of items matching the attributes. An empty list either means that
     *         no items were found or that an error occurred (eg. collection needs
     *         unlocking before listing the items).
     * @todo error
     */
    virtual BackendReturn<QList<BackendItem*> > searchItems(
        const QMap<QString, QString> &attributes) const;

    /**
     * Create a job for unlocking this collection.
     */
    virtual UnlockCollectionJob *createUnlockJob(const CollectionUnlockInfo &unlockInfo);

    /**
     * Create a job for locking this collection.
     */
    virtual LockCollectionJob *createLockJob();

    /**
     * Create a job for deleting this collection.
     */
    virtual DeleteCollectionJob *createDeleteJob(const CollectionDeleteInfo& deleteJobInfo);

    /**
     * Create a job for creating an item.
     *
     * @param label label to assign to the new item
     * @param attributes attributes to store for the new item
     * @param secret the secret to store
     * @param replace if true, an existing item with the same attributes
     *                will be replaced, if false no item will be created
     *                if one with the same attributes already exists
     * @param locked if true, the item will be locked after creation
     */
    virtual CreateItemJob *createCreateItemJob(const ItemCreateInfo& createInfo);

    /**
     * Create a job for changing this collection's authentication.
     */
    virtual ChangeAuthenticationCollectionJob* createChangeAuthenticationJob( const Peer& );

    /**
     * Get the path of the ksecret file the collection is stored inside.
     *
     * @return the path of the collection file
     */
    const QString &path() const;

    /**
     * This member is called during collection creation to store the creator application path.
     * If the collection aloready has a creator set, then this methid has no effect
     * @param exePath Path to the application that created this collection
     * @return false if the collection aloready has a creator set
     */
    bool setCreatorApplication(QString exePath);
    
    /**
     * This method returns the executable path of the application which originally
     * created this collection.
     */
    QString creatorApplication() const {
        if ( isLocked() )
            return m_secret.m_creatorApplication;
        else 
            return "";
    }

private Q_SLOTS:
    /**
     * Remove an item from our list of known items.
     *
     * @param item Item to remove
     */
    void slotItemDeleted(BackendItem *item);

    /**
     * This slot is called whenever an Item's attributes change to rebuild the item lookup
     * hashes the collection uses to search items.
     *
     * @param item Item whose attributes changed
     */
    void changeAttributeHashes(KSecretItem *item);
    
    /**
     * This slot can be called to start the sync timer (if it's not running
     * yet). It's supposed to be called after every change to the collection
     * or one of its items.
     */
    void startSyncTimer();
    
    void slotItemChanged(BackendItem *item);
    
    /**
     * This slot is called by the sync timer to sync the collection
     * on changes.
     */
    void sync();
    
public:
    /**
     * This methode get the application permission set by the user and stored into the 
     * backend file, or PermissionUndefined if the application is not found in the stored list
     * @param path complete path to the executable to be chedked
     * @return PermssionUndefined if the executable is not yet referenced by the collection or the
     * value stored by setApplicationPermission 
     */
    virtual ApplicationPermission applicationPermission(const QString& path) const;
    
    /**
     * Store the permission for the give application executable
     * @param path complete path to the executable
     * @param perm the permission to be stored for this executable
     * @return true if the application permission was accepted. The permission may not be accepted
     * if the executable given in the path parameter does not exist on the disk
     */
    virtual bool setApplicationPermission(const QString& path, ApplicationPermission perm);

protected:
    /**
     * Try to unlock the collection using the password provided.
     *
     * @return true if unlocking succeeded, false else
     * @remarks this is used by KSecretUnlockCollectionJob
     */
    BackendReturn<bool> tryUnlockPassword(const QCA::SecureArray &password);

    /**
     * Lock this collection. This is implemented here for convenience purposes.
     *
     * @remarks this is used by KSecretLockCollectionJob
     */
    BackendReturn<bool> lock();

    /**
     * Create a new item inside this collection.
     *
     * @remarks this is used by KSecretCreateItemJob
     */
    BackendReturn<BackendItem*> createItem(const QString &label,
                                           const QMap<QString, QString> &attributes,
                                           const QCA::SecureArray &secret, 
                                           const QString& contentType,
                                           bool replace,
                                           bool locked);

    /**
     * Delete this collection and emit the corresponding signals.
     *
     * @remarks this is used by KSecretDeleteCollectionJob
     */
    BackendReturn<bool> deleteCollection();

private:
    friend class KSecretUnlockCollectionJob;
    friend class KSecretLockCollectionJob;
    friend class KSecretDeleteCollectionJob;
    friend class KSecretCreateItemJob;
    friend class KSecretCreateCollectionJob;


    /**
     * Try to unlock using the currently set symmetric key.
     *
     * @return true if unlocking succeeded, false if it failed
     * @remarks this is used by KSecretUnlockCollectionJob
     */
    BackendReturn<bool> tryUnlock();

    /**
     * Serialize this ksecret collection back to a KSecretFile.
     *
     * @param errorMessage set if there's an replaceerror
     * @return true on success, false in case of an error
     */
    bool serialize(QString &errorMessage) const;

    /**
     * Set or unset the dirty flag. When setting the flag, the syncTimer will be started
     * but only of the collection is in the unlocked state
     * @param dirty the dirty value to give
     */
    void setDirty( bool dirty = true );

    // flag that's set when the collection is locked
    QString m_path; // path of the ksecret file on disk
    bool m_locked;
    KSecretEncryptionFilter *m_encryptionFilter;
    
    // flag which is set when the collection was changed and which
    // denotes that the collection should be synced back to disk.
    mutable bool m_dirty;
    // timer for syncing the collection on changes
    QTimer m_syncTimer;

    // structure holding the public data serialized when collection
    // file is first loaded
    struct PublicData {
        QString m_id;
        QString m_label;
    };
    
    // this is the secret data that's goes in the storage
    // this gets loaded when unlocking the collection
    struct SecretData {
        SecretData();
        virtual ~SecretData();
        
        QDateTime m_created;
        QDateTime m_modified;

        // configuration values
        bool m_cfgCloseScreensaver;     // close when the screensaver starts
        bool m_cfgCloseIfUnused;        // close when the last application stops using it
        quint32 m_cfgCloseUnusedTimeout; // timeout the collection will be closed after if unused (secs)

        // the acls message authentication code
        QHash<QString, ApplicationPermission> m_acls;
        QString m_creatorApplication;

        QMultiHash< QString, KSecretItem* > m_lookupHashes;
        
//         // maps lookup attribute hashes to items
//         QMultiHash<QByteArray, KSecretItem*> m_itemHashes;
//         // maps item ids to their hashes for changing/removal
//         QHash<KSecretItem*, QSet<QByteArray> > m_reverseItemHashes;

        // maps item identifiers to items
        QHash<QString, KSecretItem*> m_items;
        
    };
    
    friend KSecretStream& operator << ( KSecretStream&, const PublicData& );
    friend KSecretStream& operator >> ( KSecretStream&, PublicData& );
    friend KSecretStream& operator << ( KSecretStream&, const SecretData& );
    friend KSecretStream& operator >> ( KSecretStream&, SecretData& );
    
    PublicData  m_pub;
    SecretData  m_secret;
};

KSecretStream & operator << ( KSecretStream& out, const KSecretCollection::PublicData &data );
KSecretStream & operator >> ( KSecretStream& in, KSecretCollection::PublicData &data );
KSecretStream & operator << ( KSecretStream& out, const KSecretCollection::SecretData &data );
KSecretStream & operator >> ( KSecretStream& in, KSecretCollection::SecretData &data );


#endif
