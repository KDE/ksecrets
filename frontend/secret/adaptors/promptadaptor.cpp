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

#include "promptadaptor.h"
#include "../prompt.h"

namespace orgFreedesktopSecret
{

PromptAdaptor::PromptAdaptor(PromptBase *prompt)
    : QDBusAbstractAdaptor(prompt), m_prompt(prompt)
{
    Q_ASSERT(prompt);

    connect(prompt, SIGNAL(completed(bool,QVariant)), SLOT(slotCompleted(bool,QVariant)));
}

void PromptAdaptor::Prompt(const QString &windowId)
{
    m_prompt->prompt(windowId);
}

void PromptAdaptor::Dismiss()
{
    m_prompt->dismiss();
}

void PromptAdaptor::slotCompleted(bool dismissed, const QVariant &result)
{
    emit Completed(dismissed, QDBusVariant(result));
}

}

#include "promptadaptor.moc"
