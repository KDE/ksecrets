/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
 * Copyright 2011, Valentin Rusu <kde@rusu.info>
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

#ifndef KSECRETFILE_H
#define KSECRETFILE_H

#include <QtCore/QIODevice>
#include <QtCore/QDateTime>
#include <QtCrypto>
#include <qca_core.h>
#include <qca_basic.h>

/**
 * Encapsulates reading from and writing to a ksecret file.
 * @note KSecretFile cannot directly inherit from QFile as write
 * operations are done through a KSaveFile and read operations
 * are done through a plain QFile. 
 */
class KSecretFile
{
    static const quint32 FILE_FORMAT_VERSION;
    
public:
    /**
     * Modes for opening the ksecret file.
     */
    enum OpenMode {
        Read,  /// Open file for reading
        Write  /// Open file for writing
    };

    /**
     * Constructor.
     *
     * @param device device to read the contents from
     * @param mode set whether to read or to write
     */
    KSecretFile(QIODevice *device, OpenMode mode);

    /**
     * Destructor.
     */
    ~KSecretFile();

    /**
     * Check if the secret file was constructed validly.
     * This DOES NOT check if the file contents are valid.
     *
     * @return true if the object can be worked with, false if an
     *         error occurred during opening or reading/writing data.
     */
    bool isValid() const;
    
    OpenMode mode() const;

    /**
     * Close this ksecret file.
     */
    void close();

    /**
     * Get the current position within the file.
     *
     * @return the current file position
     */
    qint64 pos() const;

    /**
     * Set the current position within the file to pos.
     *
     * @param pos position to seek inside the underlying device
     * @return true on success, false on error
     */
    bool seek(qint64 pos);


    /**
     * Read a UINT from the file.
     *
     * @param value pointer the UINT will be written to
     * @return true if reading was successful, false else
     */
    bool readUint(quint32 *value);

    /**
     * Write a UINT to the file.
     *
     * @param value the UINT that should be written
     * @return true if writing was successful, false else
     */
    bool writeUint(quint32 value);

    /**
     * Read a DATETIME from the file.
     *
     * @param value pointer the DATETIME will be written to
     * @return true if reading was successful, false else
     */
    bool readDatetime(QDateTime *value);

    /**
     * Write a DATETIME to the file.
     *
     * @param value DATETIME to be written
     * @return true if writing was successful, false else
     */
    bool writeDatetime(const QDateTime &value);

    /**
     * Read a BYTEARRAY from the file.
     *
     * @param value pointer the BYTEARRAY will be written to
     * @return true if reading was successful, false else
     */
    bool readBytearray(QByteArray *value);

    /**
     * Write a BYTEARRAY to the file.
     *
     * @param value BYTEARRAY to be written
     * @return true if writing was successful, false else
     */
    bool writeBytearray(const QByteArray &value);

    /**
     * Read a STRING from the file.
     *
     * @param value pointer the STRING will be written to
     * @return true if reading was successful, false else
     */
    bool readString(QString *value);

    /**
     * Write a STRING to the file.
     *
     * @param value the STRING to be written
     * @return true if writing was successful, false else
     */
    bool writeString(const QString &value);

    /**
     * Read a secret BYTEARRAY/STRING from the file.
     *
     * @param value pointer the secret will be written to
     * @return true if reading was successul, false else
     * @remarks basically the same as \sa readBytearray
     *          but writes the results into a SecureArray.
     */
    bool readSecret(QCA::SecureArray *value);

    /**
     * Write a secret BYTEARRAY/STRING to the file.
     *
     * @param value the secret to be written
     * @return true if writing was successful, false else
     */
    bool writeSecret(const QCA::SecureArray &value);

    KSecretFile & operator << ( const QCA::SecureArray & );

   
private:
    bool writeHeader();
    bool readHeader();
    
    QIODevice           *m_device;
    OpenMode            m_mode;
    bool                m_valid;

    QByteArray          m_readBuffer;
};

#endif
