/*
 * Copyright 2012, Valentin Rusu <valir@kde.org>
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

#include "secretsyncapplet.h"

#include <Plasma/Containment>
#include <Plasma/ToolTipManager>

#include <QPainter>

#include <kicon.h>

K_EXPORT_PLASMA_APPLET(secretsync, SecretSyncApplet)


SecretSyncApplet::SecretSyncApplet(QObject* parent, const QVariantList& args): 
    PopupApplet(parent, args),
    m_panelContainment( true )
{
    setPopupIcon( QLatin1String("kwalletmanager") );
    setBackgroundHints( DefaultBackground );
    setMinimumSize( 100, 100 );
}

SecretSyncApplet::~SecretSyncApplet()
{
    if ( hasFailedToLaunch() ) {
        // TODO: cleanup (if needed) after launch failure
    }
    else {
        // TODO: save settings
    }
}

void SecretSyncApplet::init()
{
    Plasma::Containment * c = containment();

    if (c && (c->containmentType() == Plasma::Containment::PanelContainment ||
              c->containmentType() == Plasma::Containment::CustomPanelContainment)) {
        Plasma::ToolTipManager::self()->registerWidget(this);
        m_panelContainment = true;
    } else {
        m_panelContainment = false;
    }

    int s = 16;
    m_pixmap = KIcon("kwalletmanager").pixmap(s, s);
}

void SecretSyncApplet::paintInterface(QPainter* p, const QStyleOptionGraphicsItem* option, const QRect& contentsRect)
{
    Q_UNUSED( p );
    Q_UNUSED( option );
    
    if ( !isIconified() ) {
        /* To make applet's size matches the popup's size. The applet is the tray icon, which is 16x16 pixels size by default.*/
        adjustSize();
        return;
    }

    /* This code is borrowed from networkmanager applet */
//     int s = qMin(contentsRect.width(), contentsRect.height());
//     QRect rect(0, 0, s, s);
//     QPixmap newIcon(QSize(s, s));
//     newIcon.fill(Qt::transparent);
//     QPainter painter;
//     painter.begin(&newIcon);
// 
//     painter.drawPixmap(QPoint(0,0), m_pixmap);
// 
//     paintStatusOverlay(&painter, rect);
//     painter.end();
//     setPopupIcon(newIcon);
    
}

void SecretSyncApplet::paintStatusOverlay(QPainter* p, QRect& rect)
{
    // TODO:
}


#include "secretsyncapplet.moc"
