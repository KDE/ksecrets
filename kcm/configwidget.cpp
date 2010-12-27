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
#include "addcomputerdialog.h"
#include "ksecretsynccfg.h"

#include <kaction.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <QTableView>
#include <QTimer>
#include <QCheckBox>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kdebug.h>
#include <QStandardItemModel>
#include <QListWidgetItem>
#include <QListWidget>
#include <kconfigdialogmanager.h>
#include <kdebug.h>

ConfigWidget::ConfigWidget(QWidget* parent, Qt::WindowFlags f): 
    QWidget(parent, f),
    _mainWindow( parent )
{
    setupUi( this );
    _findComputerBtn->setVisible( false );
    _announceComputerBtn->setVisible( false );
    
    createActions();
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::createActions()
{
//     Q_ASSERT( _mainWindow != 0 );
    
    connect( _addComputerBtn, SIGNAL(clicked()), this, SLOT(onAddComputer()));
    connect( _deleteComputerBtn, SIGNAL(clicked()), this, SLOT(onDeleteComputer()));
}

void ConfigWidget::onAddComputer()
{
    AddComputerDialog dlg( this );
    if ( dlg.exec() == QDialog::Accepted ) {
        QString computerName = dlg.computerName();
        if ( _computerList->findItems( computerName, Qt::MatchExactly ).count() >0 ) {
            KMessageBox::error( this, i18n("The computer '%1' is already present into the computer list", computerName ) );
        }
        else {
            _computerList->addItem( computerName );
            emit computerListChanged();
        }
    }
}

void ConfigWidget::onDeleteComputer()
{
    if ( _computerList->currentItem() ) {
        QString message = QString( i18n("Really delete '%1' computer from the list?", 
                                        _computerList->currentItem()->text() ) );
        if ( KMessageBox::questionYesNo( this, message ) == KMessageBox::Yes ) {
            delete _computerList->takeItem( _computerList->currentRow() );
            emit computerListChanged();
        }
    }
}

void ConfigWidget::load(KSecretSyncCfg* settings)
{
    kDebug() << "ConfigWidget::load";
    _computerList->clear();
    foreach ( QString computerName, settings->computerList() ) {
        _computerList->addItem( computerName );
    }
}

void ConfigWidget::save(KSecretSyncCfg* settings)
{
    kDebug() << "ConfigWidget::save";

    QStringList computerList;
    if ( _computerList->count() >0 ) {
        for ( int r = 0; r < _computerList->count(); r++ ) {
            QListWidgetItem* item = _computerList->item( r );
            computerList.append( item->text() );
        }
    }
    KCoreConfigSkeleton::ItemStringList *computerListItem = 
            dynamic_cast< KCoreConfigSkeleton::ItemStringList*>( settings->findItem( QLatin1String("computerList") ) );
    Q_ASSERT( computerListItem != 0);
    computerListItem->setValue( computerList );
    settings->writeConfig();
}

void ConfigWidget::defaults()
{
    _computerList->clear();
}

void ConfigWidget::enableSyncToggled(bool syncEnabled)
{
    kcfg_syncInterval->setEnabled( syncEnabled );
    _intervalLabel->setEnabled( syncEnabled );
    _intervalUnits->setEnabled( syncEnabled );
}

void ConfigWidget::onSynchIntervalChanged(int )
{
}

#include "configwidget.moc"
