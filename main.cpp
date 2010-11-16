/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
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

#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <QtDBus/QDBusConnection>
#include <QtCrypto/QtCrypto>

#include "backend/backendmaster.h"
#include "backend/temporary/temporarycollectionmanager.h"
#include "frontend/secret/service.h"
#include "ui/dialoguimanager.h"

int main(int argc, char **argv)
{
    KAboutData aboutdata("ksecretservice", 0, ki18n("KDE Secret Service"),
                         "0.0", ki18n("KDE Secret Service"),
                         KAboutData::License_GPL, ki18n("(C) 2010 Michael Leupold"));
    aboutdata.addAuthor(ki18n("Michael Leupold"), ki18n("Maintainer"), "lemma@confuego.org");
    aboutdata.addCredit(ki18n("Valentin Rusu"), ki18n("ACL Handling"), "vrusu@fsfe.org");
    aboutdata.setProgramIconName("ksecretservice");

    KCmdLineArgs::init(argc, argv, &aboutdata);
    KUniqueApplication::addCmdLineOptions();
    KUniqueApplication app;

    app.setQuitOnLastWindowClosed(false);

    if(!KUniqueApplication::start()) {
        kDebug() << "ksecretservice is already running!";
        return 0;
    }

    if(!QDBusConnection::sessionBus().registerService("org.freedesktop.Secret")) {
        kDebug() << "Couldn't register org.freedesktop.Secret D-Bus service!";
        return 1;
    }

    // initialize QCA
    QCA::Initializer qcaInit;

    BackendMaster *master = BackendMaster::instance();
    master->setUiManager(new DialogUiManager);
    master->addManager(new TemporaryCollectionManager(master));
    Service service(BackendMaster::instance());

    return app.exec();
}
