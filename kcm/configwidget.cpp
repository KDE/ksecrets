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
#include "configconstants.h"

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
#include <QStandardItemModel>


ConfigWidget::ConfigWidget(QWidget* parent, Qt::WindowFlags f): 
    QWidget(parent, f),
    _mainWindow( parent ),
    _saveTimer(0)
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
    delete _saveTimer;
}

void ConfigWidget::createActions()
{
    Q_ASSERT( _mainWindow != 0 );
    
    connect( _addComputerBtn, SIGNAL(clicked()), this, SLOT(onAddComputer()));
    connect( _deleteComputerBtn, SIGNAL(clicked()), this, SLOT(onDeleteComputer()));
}

void ConfigWidget::onAddComputer()
{
    AddComputerDialog dlg( this );
    if ( dlg.exec() == QDialog::Accepted ) {
        // TODO: check if entered computer name was not already configured
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
    mainGroup.writeEntry(MAIN_CURRENT_TAB, _tabs->currentIndex());
    
    mainGroup.sync();
}

void ConfigWidget::loadSettings()
{
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup mainGroup( config, "main" );

    int currentTab = mainGroup.readEntry<int>(MAIN_CURRENT_TAB, 0);
    _tabs->setCurrentIndex( currentTab );
    
    bool enableSync = mainGroup.readEntry<bool>(MAIN_ENABLE_SYNC_ENTRY, false);
    _enableSync->setChecked( enableSync );
    enableSyncToggled( enableSync );
    
    int syncInterval = mainGroup.readEntry<int>(MAIN_SYNC_INTERVAL, MIN_SYNC_INTERVAL );
    if ( syncInterval < MIN_SYNC_INTERVAL ) {
        kDebug() << "Fixing too short syncInterval from " << syncInterval << " to " << MIN_SYNC_INTERVAL;
        syncInterval = MIN_SYNC_INTERVAL;
    }
    _intervalSpinBox->setValue( syncInterval );
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

void ConfigWidget::createLogEntry(const QString& )
{
    // TODO: implement logging
}

void ConfigWidget::onTabChanged(int tabIndex)
{
    // TODO: react to tab changes
}

#include "configwidget.moc"
