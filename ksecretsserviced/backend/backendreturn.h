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

#ifndef BACKENDRETURN_H
#define BACKENDRETURN_H

#include <QtCore/QString>

/**
  * Error codes for backend calls.
  */
enum ErrorType {
    BackendNoError,               /// no error
    BackendErrorOther,            /// an unspecified error
    BackendErrorAlreadyExists,    /// a collection could not be created because one with
    /// the same name already exists
    BackendErrorIsLocked,         /// the object (item/collection) must be unlocked before this
    /// call can be made
    BackendErrorNotSupported,     /// the backend doesn't support calling this method
    BackendErrorAclSetPermission /// the permission could not be set
};


/**
 * Possible Erroneous Return encapsulates return values and an error code/message
 * as a method's return value.
 *
 * @todo it might be desirable to have BackendReturn implicitly shared but it shouldn't hurt too
 *       much as long as value is implicitly shared.
 */
template <typename T>
class BackendReturn
{
public:
    BackendReturn() :
        m_error( BackendNoError ) {
    }
    
    /**
     * Constructor.
     */
    explicit BackendReturn(const T &value, ErrorType error = BackendNoError,
                  const QString &errorMessage = QString())
        : m_value(value), m_error(error), m_errorMessage(errorMessage) {
    }

    /**
     * Copy constructor.
     */
    BackendReturn(const BackendReturn<T> &other)
        : m_value(other.m_value), m_error(other.m_error), m_errorMessage(other.m_errorMessage) {
    }

    /**
     * Get the enclosed value of the object
     */
    const T &value() const {
        return m_value;
    }
    
    BackendReturn &operator = ( const T& value ) {
        m_value = value;
        return *this;
    }

    /**
     * Check if this return object contains an error.
     *
     * @return true if this return object contains an error, false if it doesn't contain
     *         an error
     */
    bool isError() const {
        return m_error != BackendNoError;
    }

    /**
     * Get the error enclosed in the object
     */
    ErrorType error() const {
        return m_error;
    }

    /**
     * Get the error message enclosed in the object
     */
    const QString &errorMessage() const {
        return m_errorMessage;
    }
    
    void setError( ErrorType errType, const QString& msg ) {
        m_error = errType;
        m_errorMessage = msg;
    }

private:
    T m_value;
    ErrorType m_error;
    QString m_errorMessage;
};

/**
 * Specialized template for return objects not bearing a value.
 */
template <>
class BackendReturn<void>
{
public:
    /**
     * Constructor.
     */
    BackendReturn(ErrorType error = BackendNoError, const QString &errorMessage = QString())
        : m_error(error), m_errorMessage(errorMessage) {
    }

    /**
     * Check if this return object contains an error.
     *
     * @return true if this return object contains an error, false if it doesn't contain
     *         an error
     */
    bool isError() const {
        return m_error != BackendNoError;
    }

    /**
     * Get the error enclosed in the object
     */
    ErrorType error() const {
        return m_error;
    }

    /**
     * Get the error message enclosed in the object
     */
    const QString &errorMessage() const {
        return m_errorMessage;
    }

    void setError( ErrorType errType, const QString& msg ) {
        m_error = errType;
        m_errorMessage = msg;
    }
    
private:
    ErrorType m_error;
    QString m_errorMessage;
};

#endif
