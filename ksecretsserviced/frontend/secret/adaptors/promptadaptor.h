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

#ifndef ORG_FREEDESKTOP_SECRET_PROMPTADAPTOR_H
#define ORG_FREEDESKTOP_SECRET_PROMPTADAPTOR_H

#include "../prompt.h"

#include <QtDBus/QDBusAbstractAdaptor>
#include <QDBusContext>

namespace orgFreedesktopSecret
{

/**
 * D-Bus adaptor class for Prompt objects.
 */
class PromptAdaptor : public QDBusAbstractAdaptor, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Secret.Prompt")

public:
    /**
     * Constructor.
     *
     * @param prompt prompt object to attach the adaptor to
     */
    PromptAdaptor(PromptBase *prompt);

public Q_SLOTS:
    void Prompt(const QString &windowId);

    void Dismiss();

Q_SIGNALS:
    void Completed(bool dismissed, QDBusVariant result);

private Q_SLOTS:
    void slotCompleted(bool dismissed, const QVariant &result);

private:
    PromptBase *m_prompt;
};

}

#endif
