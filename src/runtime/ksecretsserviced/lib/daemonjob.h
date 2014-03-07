/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef DAEMONJOB_H
#define DAEMONJOB_H

#include <KCompositeJob>

class DaemonJob : public KCompositeJob {
    Q_OBJECT
    
public:
    explicit DaemonJob(QObject *parent =0);
    
    bool isFinished() const;
    
    /**
     * This overrides KCompositeJob::addSubjob only to make it public
     */
    bool addSubjob(KJob*) Q_DECL_HIDDEN;

private Q_SLOTS:
    void slotJobFinished(KJob*);
    
private:
    bool m_finished;
};

#endif // DAEMONJOB_H
