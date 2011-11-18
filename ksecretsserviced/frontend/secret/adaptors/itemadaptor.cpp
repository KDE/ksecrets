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

#include "itemadaptor.h"
#include "../item.h"
#include "daemonsecret.h"

namespace orgFreedesktopSecret
{

ItemAdaptor::ItemAdaptor(Item *item)
    : QDBusAbstractAdaptor(item), m_item(item)
{
    Q_ASSERT(item);
}

bool ItemAdaptor::locked() const
{
    return m_item->locked();
}

void ItemAdaptor::setAttributes(const StringStringMap &attributes)
{
    m_item->setAttributes(attributes);
}

StringStringMap ItemAdaptor::attributes() const
{
    return m_item->attributes();
}

void ItemAdaptor::setLabel(const QString &label)
{
    m_item->setLabel(label);
}

QString ItemAdaptor::label() const
{
    return m_item->label();
}

qulonglong ItemAdaptor::created() const
{
    return m_item->created();
}

qulonglong ItemAdaptor::modified() const
{
    return m_item->modified();
}

QDBusObjectPath ItemAdaptor::Delete()
{
    return m_item->deleteItem();
}

SecretStruct ItemAdaptor::GetSecret(const QDBusObjectPath &session)
{
    return m_item->getSecret(session);
}

void ItemAdaptor::SetSecret(const SecretStruct &secret)
{
    m_item->setSecret( secret );
}

}

#include "itemadaptor.moc"
