/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <valir@kde.org>
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

#ifndef KSECRETSAPPJOB_H
#define KSECRETSAPPJOB_H

#include <kcompositejob.h>
#include <QVariant>

/**
 * This class extends KCompositeJob for several reasons:
 * - make addSubjob public
 * - implement start() which is virtual abstract
 * - automatically emitResult() upon last job exit
 */
class KSecretsAppJob : public KCompositeJob {
    Q_OBJECT
public:
    explicit KSecretsAppJob(QObject* parent = 0);
    
    virtual bool addSubjob( KJob* );
    void start();
    
    void setCustomData( const QVariant& customData ) { m_customData = customData; }
    const QVariant& customData() const { return m_customData; }
    
protected:
    virtual void slotResult( KJob* );
    
private:
    QVariant    m_customData;
};

#endif // KSECRETSAPPJOB_H
