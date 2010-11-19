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

#include <kstatusnotifieritem.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kicon.h>
#include <klocalizedstring.h>

#include "trayicon.h"

TrayIcon::TrayIcon(QObject* parent): QObject(parent)
{
    _trayIcon = new KStatusNotifierItem( parent );
    _trayIcon->setObjectName( QLatin1String("ksecretsync tray" ));
    _trayIcon->setCategory( KStatusNotifierItem::SystemServices );
    _trayIcon->setStatus( KStatusNotifierItem::Passive );
    _trayIcon->setIconByName(QLatin1String( "ksecretsync-idle" ));
    _trayIcon->setToolTip( QLatin1String( "ksecretsync idle" ), i18n("KDE Wallet"), i18n("Idle"));
}

void TrayIcon::setupActions(KActionCollection* actionCollection)
{
    KAction *action = actionCollection->addAction( QLatin1String("ksecretsync settings") );
    action->setText( i18n("&Configure KSecretSync...") );
    action->setIcon( KIcon( QLatin1String( "configure" )) );
}
