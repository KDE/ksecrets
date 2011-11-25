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

#include "ksecretsapp.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>


int main(int argc, char **argv)
{
    // FIXME: what version should we put here?
    KAboutData aboutdata("ksecrets", 0, ki18n("ksecrets"),
                         "0.1",
                         ki18n("Secrets Service Handling Tool"),
                         KAboutData::License_GPL, ki18n("(C) 2011 Valentin Rusu"));
    aboutdata.addAuthor(ki18n("Valentin Rusu"), ki18n("Maintainer"), "kde@rusu.info");
    aboutdata.setProgramIconName("ksecretsservice");

    KLocale::setMainCatalog("ksecrets_ksecrets");
    
    KCmdLineArgs::init(argc, argv, &aboutdata);
    
    KCmdLineOptions options;
    options.add( "lc", ki18n("List existing secret collection") );
    options.add( "l <collection>", ki18n("List the contents of the collection named <secret collection>") );
    KCmdLineArgs::addCmdLineOptions( options );

    KSecretsApp app;
    return app.exec();
}

