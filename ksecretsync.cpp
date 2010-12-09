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

#include "ksecretsync.h"
#include "trayicon.h"
#include "statuswidget.h"

#include <kconfig.h>
#include <klocalizedstring.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kmessagebox.h>

KSecretSync::KSecretSync(QWidget* parent, Qt::WindowFlags f): 
    KXmlGuiWindow(parent, f)
{

    KConfig cfg( QLatin1String( "ksecretsyncrc" )); 
    // TODO: read settings here
    
    _trayIcon = new TrayIcon( this );
    
    _statusWidget = new StatusWidget( this );
    setCentralWidget( _statusWidget );
    
    createGUI( QLatin1String( "ksecretsync.rc" ) );
    
    KAction *action;
    
    action = createAction( QLatin1String("ksecretsync synchronize now") );
    action->setText( i18n("&Synchronize now") );
    action->setIcon( KIcon( QLatin1String( "view-refresh" ) ) );
    connect( action, SIGNAL(triggered(bool)), this, SLOT(onSynchronizeNow(bool)));
}

KSecretSync::~KSecretSync()
{
}

void KSecretSync::closeEvent(QCloseEvent* event)
{
    delete _trayIcon;
    KMainWindow::closeEvent( event );
}

KAction *KSecretSync::createAction( const QLatin1String &description )
{
    KAction* action = actionCollection()->addAction( description );
    if ( _trayIcon )
        _trayIcon->addAction( action );
    return action;
}

void KSecretSync::createLogEntry(const QString& )
{
    // TODO: create log entry here
}

void KSecretSync::onSynchronizeNow(bool )
{
    // TODO: implement this
    KMessageBox::information( this, i18n( "This function is not yet implemented" ) );
}


#include "ksecretsync.moc"
