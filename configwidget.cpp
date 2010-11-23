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

#include "configwidget.h"
#include "ksecretsync.h"
#include "addcomputerdialog.h"

#include <qevent.h>
#include <kaction.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTimer>

/**
 * This is the save timer interval default value
 * The save timer is started whenever a setting is changed and allow for it to be
 * persisted into the configuration file
 */
#define SAVE_TIMER_INTERVAL 2000

ConfigWidget::ConfigWidget(KSecretSync* parent, Qt::WindowFlags f): 
    QWidget(parent, f),
    _mainWindow( parent )
{
    setupUi( this );
    _findComputerBtn->setVisible( false );
    _announceComputerBtn->setVisible( false );
    
    createActions();
}

void ConfigWidget::createActions()
{
    Q_ASSERT( _mainWindow != 0 );
    KAction *action;
    
    action = _mainWindow->createAction( QLatin1String("ksecretsync synchronize now") );
    action->setText( i18n("&Synchronize now") );
    action->setIcon( KIcon( QLatin1String( "view-refresh" ) ) );
    connect( action, SIGNAL(triggered(bool)), this, SLOT(onSynchronizeNow(bool)));
    
    connect( _addComputerBtn, SIGNAL(clicked()), this, SLOT(onAddComputer()));
    connect( _deleteComputerBtn, SIGNAL(clicked()), this, SLOT(onDeleteComputer()));
    connect( _synchronizeNowBtn, SIGNAL(clicked()), this, SLOT(onSynchronizeNow()));
}

void ConfigWidget::onSynchronizeNow(bool )
{
    // TODO: implement this
    KMessageBox::information( this, i18n( "This function is not yet implemented" ) );
}

void ConfigWidget::onAddComputer()
{
    AddComputerDialog dlg( this );
    if ( dlg.exec() == QDialog::Accepted ) {
        _computerList->addItem( dlg._computerName );
    }
}

void ConfigWidget::onDeleteComputer()
{
    if ( _computerList->currentItem() ) {
        QString message = QString( i18n("Really delete '%1' computer from the list?", 
                                        _computerList->currentItem()->text() ) );
        if ( KMessageBox::questionYesNo( this, message ) == KMessageBox::Yes ) {
            delete _computerList->takeItem( _computerList->currentRow() );
        }
    }
}

void ConfigWidget::saveSettingsLater()
{
    if ( _saveTimer == 0 ) {
        _saveTimer = new QTimer(this);
        _saveTimer->setSingleShot(true);
        connect( _saveTimer, SIGNAL(timeout()), SLOT(onSaveTimer()) );
        _saveTimer->setInterval( SAVE_TIMER_INTERVAL );
    }
    
    _saveTimer->start();
}

void ConfigWidget::onSaveTimer()
{
    // TODO: implement save settings
}

#include "configwidget.moc"
