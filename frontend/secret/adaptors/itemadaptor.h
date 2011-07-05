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

#ifndef ORG_FREEDESKTOP_SECRET_ITEMADAPTOR_H
#define ORG_FREEDESKTOP_SECRET_ITEMADAPTOR_H

#include "dbustypes.h"

#include <QtDBus/QDBusAbstractAdaptor>
#include <QtCore/QString>
#include <QtCore/QMap>

class Item;

namespace orgFreedesktopSecret
{

/**
 * D-Bus adaptor class for Item objects.
 */
class ItemAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Secret.Item")
    Q_PROPERTY(bool Locked READ locked)
    Q_PROPERTY(StringStringMap Attributes READ attributes WRITE setAttributes)
    Q_PROPERTY(QString Label READ label WRITE setLabel)
    Q_PROPERTY(qulonglong Created READ created)
    Q_PROPERTY(qulonglong Modified READ modified)

public:
    /**
     * Constructor.
     *
     * @param item item object to attach the adaptor to
     */
    ItemAdaptor(Item *item);

    bool locked() const;

    void setAttributes(const StringStringMap &attributes);

    StringStringMap attributes() const;

    void setLabel(const QString &label);

    QString label() const;

    qulonglong created() const;

    qulonglong modified() const;

public Q_SLOTS:
    QDBusObjectPath Delete();

    SecretStruct GetSecret(const QDBusObjectPath &session);

    void SetSecret(const SecretStruct &secret);

private:
    Item *m_item;
};

}

#endif
