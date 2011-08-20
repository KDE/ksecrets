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

#include "addcomputerdialog.h"
#include <klocalizedstring.h>


AddComputerDialog::AddComputerDialog(QWidget* parent, Qt::WFlags flags): 
    KDialog(parent, flags)
{
    setCaption( i18n("Add computer") );
    setButtons( KDialog::Ok | KDialog::Cancel );
    _widget = new AddComputerDialogWidget(this);
    setMainWidget( _widget );
    connect( _widget->_computerName, SIGNAL(textChanged(QString)), SLOT(computerNameChanged(QString)) );
    enableButtonOk(false);
    _widget->setFocus();
    // TODO: add more port validation to this dialog to complete the existing input mask validation
}

void AddComputerDialog::computerNameChanged(const QString& computerName )
{
    if ( computerName.length() >0 ) {
        enableButtonOk(true);
    }
    else
        enableButtonOk(false);
}

QString AddComputerDialog::computerName() const
{
    return QString("%1:%2").arg( _widget->_computerName->text() ).arg( _widget->_port->text() );
}


#include "addcomputerdialog.moc"
