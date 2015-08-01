/*
 * Copyright 2010, Dario Freddi <dario.freddi@collabora.co.uk>
 * Copyright 2011, Valentin Rusu <valir@kde.org>
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


#ifndef KWLIMPORTERJOB_H
#define KWLIMPORTERJOB_H

#include <KCompositeJob>

class KUiServerJobTracker;

class KwlImporterJob : public KCompositeJob
{
    Q_OBJECT
    Q_DISABLE_COPY(KwlImporterJob)

public:
    explicit KwlImporterJob( QObject* parent = 0);
    virtual ~KwlImporterJob();

    /*
     * This methide scans the users personal settings directory and 
     * return true if it founds *.kwl any files inside
     * @return true if the user has *.kwl files
     */
    static bool userHasWallets();

public Q_SLOTS:
    virtual void start();

protected Q_SLOTS:
    virtual bool doKill();
    virtual bool doSuspend();
    virtual bool doResume();
    virtual void slotResult(KJob* job);

private Q_SLOTS:
    void run();

private:
    KUiServerJobTracker *_jobTracker;
};

#endif // KWLIMPORTERJOB_H
