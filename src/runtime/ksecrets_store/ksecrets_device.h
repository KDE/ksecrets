/*
    This file is part of the KDE Libraries

    Copyright (C) 2015 Valentin Rusu (valir@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSECRETS_DEVICE_H
#define KSECRETS_DEVICE_H

#include <memory>
/**
 * @brief This class acts as an interface to allow testing the CryptBuffer
 */
class KSecretsDevice {
public:
    virtual ~KSecretsDevice() = default;
    virtual const unsigned char* iv() const noexcept = 0;

    virtual bool read(void* buf, size_t count) noexcept = 0;
    template <typename T> bool read(T& s) noexcept
    {
        return read(&s, sizeof(s));
    }

    virtual bool write(const void* buf, size_t count) noexcept = 0;
    template <typename T> bool write(T t) noexcept
    {
        return write(&t, sizeof(t));
    }
};

#endif
