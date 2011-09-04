/*
 * Copyright 2010, Dario Freddi <dario.freddi@collabora.co.uk>
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


#ifndef IMPORTSINGLEWALLETJOB_H
#define IMPORTSINGLEWALLETJOB_H

#include <QtCore/QStringList>
#include <QtDBus/QDBusObjectPath>
#include <KCompositeJob>

#include <ksecretsservicecollection.h>

class KPasswordDialog;
namespace KWallet
{
class Backend;
}

using namespace KSecretsService;

class ImportSingleWalletJob : public KCompositeJob
{
    Q_OBJECT
    Q_DISABLE_COPY(ImportSingleWalletJob)

    enum Status {
        Initializing,
        ProcessingCurrentFolder,
        ProcessingEntry,
        ProcessingNextFolder
    };

public:
    explicit ImportSingleWalletJob( const QString &walletPath, QObject* parent = 0);
    virtual ~ImportSingleWalletJob();

public Q_SLOTS:
    virtual void start();

protected Q_SLOTS:
    virtual bool doKill();
    virtual bool doResume();
    virtual bool doSuspend();

private Q_SLOTS:
    void checkImportNeeded();
    void checkImportNeededItems(KJob*);
    void checkImportNeededSecret(KJob*);
    void run();
    void onGotWalletPassword(QString, bool);
    void onWalletOpened(bool);
    void processCurrentFolder();
    void processNextEntry();
    void processNextFolder();
    void createItemFinished(KJob*);

private:
    bool canProceed();

    QString m_walletPath;
    QString m_walletName;
    KWallet::Backend *m_wallet;
    QDBusObjectPath m_collectionPath;
    bool m_isSuspended;
    bool m_hasKillRequest;

    QStringList m_folderList;
    QStringList m_currentEntryList;
    QString m_currentFolder;

    Collection *m_collection;
    Collection *m_checkCollection;
    KPasswordDialog *m_passDlg;

    Status m_status;
};

#endif // IMPORTSINGLEWALLETJOB_H
