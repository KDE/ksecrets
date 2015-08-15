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
#ifndef KSECRETS_CRYPT_H
#define KSECRETS_CRYPT_H

#include <sys/types.h>

struct CryptBuffer {
    CryptBuffer()
        : len_(0)
        , data_(nullptr)
    {
    }
    ~CryptBuffer() { delete[] data_; }

    /**
     * @brief Allocate memory in multiples of cipher block len
     *
     * The reallocation operation is non-destructive, e.g. the data is copied
     * from the old buffer into the new one.
     *
     * @param rlen is the lenght of the data that'll be handled in the buffer
     *
     * @return true if reallocation succeeded
     */
    bool resize(size_t rlen) noexcept;

    void empty() noexcept;

    /**
     * @brief Allocate an exact amount of memory
     *
     * @param rlen
     *
     * @return true if allocation succeeded
     */
    bool allocate(size_t rlen) noexcept;

    static constexpr size_t cipherBlockLen_ = 8; // blowfish block len is 8
    size_t len_;
    unsigned char* data_;
};

#endif
// vim: tw=220:ts=4
