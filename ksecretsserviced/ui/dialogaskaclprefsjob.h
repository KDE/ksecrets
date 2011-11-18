/*
 *  Copyright (C) 2010  Valentin Rusu <kde@rusu.info>
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

#ifndef DIALOGASKACLPREFSJOB_H
#define DIALOGASKACLPREFSJOB_H

#include <abstractacljobs.h>
#include <QtGui/QDialog>

class DialogAskCreateAclPrefs;

class DialogAskCreateAclPrefsJob : public AbstractAskAclPrefsJob
{
    Q_OBJECT
public:
    DialogAskCreateAclPrefsJob(AbstractUiManager* manager, const CollectionCreateInfo &jobInfo);

protected:
    virtual void start();

private Q_SLOTS:
    void dialogFinished(int);

private:
    DialogAskCreateAclPrefs *m_dialog;
};

class DialogAskUnlockAclPrefs;

class DialogAskUnlockAclPrefsJob : public AbstractAskAclPrefsJob
{
    Q_OBJECT
public:
    DialogAskUnlockAclPrefsJob(AbstractUiManager* manager, const CollectionUnlockInfo &jobInfo);

protected:
    virtual void start();
    
private Q_SLOTS:
    void dialogFinished(int);
    
private:
    DialogAskUnlockAclPrefs *m_dialog;
};

#endif // DIALOGASKACLPREFSJOB_H
