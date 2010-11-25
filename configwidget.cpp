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
#include "computersyncjob.h"
#include "syncjob.h"

#include <qevent.h>
#include <kaction.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTimer>
#include <QCheckBox>
#include <KStandardDirs>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kdebug.h>

/**
 * This is the save timer interval default value
 * The save timer is started whenever a setting is changed and allow for it to be
 * persisted into the configuration file
 */
#define SAVE_TIMER_INTERVAL 2000

#define MAIN_ENABLE_SYNC_ENTRY "enableSync"
#define MAIN_SYNC_INTERVAL "syncInterval"

ConfigWidget::ConfigWidget(KSecretSync* parent, Qt::WindowFlags f): 
    QWidget(parent, f),
    _mainWindow( parent ),
    _saveTimer(0),
    _synchTimer(0)
{
    setupUi( this );
    _findComputerBtn->setVisible( false );
    _announceComputerBtn->setVisible( false );
    
    createActions();
    
    loadSettings();
}

ConfigWidget::~ConfigWidget()
{
    // FIXME: should we wait for pending timeout signals ?
    delete _synchTimer;
    delete _saveTimer;
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

void ConfigWidget::saveGeneralSettings()
{
    saveSettingsLater();
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
    kDebug() << "onSaveTimer";
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup mainGroup( config, "main" );
    
    mainGroup.writeEntry(MAIN_ENABLE_SYNC_ENTRY, _enableSync->isChecked());
    mainGroup.writeEntry(MAIN_SYNC_INTERVAL, _intervalSpinBox->value());
    
    mainGroup.sync();
}

void ConfigWidget::onSyncTimer()
{
    kDebug() << "onSyncTimer";
    if ( _enableSync->isChecked() )
        startSync();
}

void ConfigWidget::loadSettings()
{
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup mainGroup( config, "main" );
    
    bool enableSync = mainGroup.readEntry<bool>(MAIN_ENABLE_SYNC_ENTRY, false);
    _enableSync->setChecked( enableSync );
    enableSyncToggled( enableSync );
    
    int syncInterval = mainGroup.readEntry<int>(MAIN_SYNC_INTERVAL, 1 );
    _intervalSpinBox->setValue( syncInterval );
    if ( _synchTimer == 0 ) {
        _synchTimer = new QTimer( this );
        connect( _synchTimer, SIGNAL(timeout()), SLOT(onSyncTimer()) );
    }
    _synchTimer->setInterval( syncInterval );
    _synchTimer->start();
}

void ConfigWidget::enableSyncToggled(bool syncEnabled)
{
    _saveGeneralChangesBtn->setEnabled( true );
    _intervalSpinBox->setEnabled( syncEnabled );
    _intervalLabel->setEnabled( syncEnabled );
    _intervalUnits->setEnabled( syncEnabled );
}

void ConfigWidget::onSynchIntervalChanged(int )
{
    _saveGeneralChangesBtn->setEnabled(true);
}

void ConfigWidget::onSynchNow()
{
    kDebug() << "onSynchNow";
    startSync();
}

void ConfigWidget::startSync()
{
    kDebug() << "startSync";
    if ( _computerList->count() == 0 ) {
        return;
    }
    
    SyncJob *syncJob = new SyncJob;
    foreach ( QListWidgetItem* item, _computerList->findItems("*", Qt::MatchWildcard ) ) {
        new ComputerSyncJob( syncJob, item->text(), this );
    }
    syncJob->start();
}

void ConfigWidget::createLogEntry(const QString& )
{
    // TODO: implement logging
}

#include "configwidget.moc"
