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

#ifndef ADDCOMPUTERDIALOG_H
#define ADDCOMPUTERDIALOG_H

#include <ui_addcomputerwidget.h>
#include <kdialog.h>

class AddComputerDialogWidget : public QWidget, public Ui_AddComputerWidget
{
    Q_OBJECT
public:
    AddComputerDialogWidget( QWidget* parent ) : QWidget( parent ) {
        setupUi(this);
    }
    friend class AddComputerDialog;
};

class AddComputerDialog : public KDialog
{
    Q_OBJECT
public:
    explicit AddComputerDialog(QWidget* parent = 0, Qt::WFlags flags = 0);

    QString computerName() const;
    
private Q_SLOTS:
    void computerNameChanged( const QString& );
    
private:
    AddComputerDialogWidget *_widget;
};


#endif // ADDCOMPUTERDIALOG_H
