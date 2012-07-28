/*
 * Copyright 2010, Valentin Rusu <kde@rusu.info>
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

#include "ksecretsyncwindow.h"
#include "trayicon.h"
#include "statuswidget.h"
#include "syncdaemon.h"
#include "syncmodel.h"

#include <kconfig.h>
#include <kapplication.h>
#include <klocalizedstring.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>
#include <QStandardItemModel>

KSecretSyncWindow::KSecretSyncWindow(QWidget* parent): 
    KXmlGuiWindow(parent)
{

//     _trayIcon = new TrayIcon( this );
// 
//     _syncModel = new SyncModel;
//     _syncDaemon = new SyncDaemon( this, _syncModel );
// 
//     _statusWidget = new StatusWidget( this );
//     setCentralWidget( _statusWidget );

//    _statusWidget->_computersView->setModel( _syncModel );
// 
//     connect( _statusWidget->_configureBtn, SIGNAL(clicked()), this, SLOT(onConfigure()));
//     connect( _statusWidget->_computersView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onComputerDoubleClicked(QModelIndex)));
    // TODO: connect to the status LED doubleClicked signal and allow enable state toggle from it

    KAction *action;
//     action = createAction( QLatin1String("ksecretsync synchronize now") );
//     action->setText( i18n("&Synchronize now") );
//     action->setIcon( KIcon( QLatin1String( "view-refresh" ) ) );
//     connect( action, SIGNAL(triggered(bool)), this, SLOT(onSynchronizeNow(bool)));
//     _trayIcon->addAction( action );

    action = new KAction( this );  //createAction( QLatin1String("ksecretsync_configure") );
    action->setText( i18n("&Configure...") );
//    action->setIcon( KIcon( QLatin1String( "run-build-configure") ) );
    actionCollection()->addAction( "ksecretsync_configure", action );
    connect( action, SIGNAL(triggered(bool)), this, SLOT(onConfigure(bool)));
//     _trayIcon->addAction( action );

  KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

  createGUI( QLatin1String( "ksecretsync.rc" ) );
  show();
}

// KSecretSyncWindow::~KSecretSyncWindow()
// {
//     delete _syncDaemon;
//     delete _syncModel;
// }

// void KSecretSyncWindow::closeEvent(QCloseEvent* event)
// {
//     delete _trayIcon; 
//     _trayIcon = 0;
// 
//     KMainWindow::closeEvent( event );
// }


void KSecretSyncWindow::createLogEntry(const QString& )
{
    // TODO: create log entry here
}

// void KSecretSyncWindow::onSynchronizeNow(bool )
// {
//     // TODO: implement this
//     KMessageBox::information( this, i18n( "This function is not yet implemented" ) );
// }
// 
// void KSecretSyncWindow::onConfigure(bool )
// {
//     KToolInvocation::startServiceByDesktopName( QLatin1String( "ksecretsync" ));    
// }
// 
// void KSecretSyncWindow::onComputerDoubleClicked(QModelIndex )
// {
//     if ( _syncModel->hasComputers() ) {
//         // TODO: what should we do when a computer is double clicked ?
//     }
//     else {
//         onConfigure();
//     }
// }

#include "ksecretsyncwindow.moc"
