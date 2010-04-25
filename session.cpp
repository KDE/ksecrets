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

#include "session.h"
#include "dbus/sessionadaptor.h"
#include "service.h"

#include <secrettool.h>

#include <QtDBus/QDBusConnection>
#include <QtCore/QRegExp>

#include <QtCore/QDebug>

Session::Session(Service *parent)
 : QObject(parent),
   m_objectPath(parent->objectPath().path() + "/session/" + createId()),
   m_cipher(0)
{
   // register on the bus
   new orgFreedesktopSecret::SessionAdaptor(this);
   QDBusConnection::sessionBus().registerObject(m_objectPath.path(), this);
}

Session::~Session()
{
   delete m_cipher;
}

const QDBusObjectPath &Session::objectPath() const
{
   return m_objectPath;
}

Session *Session::create(const QString &algorithm, const QVariant &input,
                         QVariant &output, const QString &peer, Service *parent)
{
   static QRegExp rxAlgorithm("^dh-ietf(\\d+)-([^-]+)-([^-]+)-([^-]+)$",
                              Qt::CaseInsensitive);
   
   Session *session = 0;
   
   if (algorithm == "plain") {
      session = new Session(parent);
      session->m_encrypted = false;
      output.setValue(QString(""));
   } else if (rxAlgorithm.exactMatch(algorithm) &&
              input.type() == QVariant::ByteArray) {
      QString encalgo = rxAlgorithm.cap(2).toLower();
      QString blockmode = rxAlgorithm.cap(3).toLower();
      QString padding = rxAlgorithm.cap(4).toLower();

      QCA::KeyGenerator keygen;
      
      // determine the discrete logarithm group to use
      QCA::DLGroupSet groupnum;
      switch (rxAlgorithm.cap(1).toInt()) {
      case 768:
         groupnum = QCA::IETF_768;
         break;
      case 1024:
         groupnum = QCA::IETF_1024;
         break;
      case 1536:
         groupnum = QCA::IETF_1536;
         break;
      case 2048:
         groupnum = QCA::IETF_2048;
         break;
      case 3072:
         groupnum = QCA::IETF_3072;
         break;
      case 4096:
         groupnum = QCA::IETF_4096;
         break;
      case 6144:
         groupnum = QCA::IETF_6144;
         break;
      case 8192:
         groupnum = QCA::IETF_8192;
         break;
      default:
         // no known discrete logarithm group
         return 0;
      }
      QCA::DLGroup dlgroup(keygen.createDLGroup(groupnum));
      if (dlgroup.isNull()) {
         return 0;
      }
      
      // determine if we support (or want to support)
      // the encryption algorithm.
      if ((encalgo == "blowfish" || encalgo == "twofish" ||
           encalgo == "aes128" || encalgo == "aes192" ||
           encalgo == "aes256") &&
           QCA::isSupported(QString("%1-%2-%3").arg(encalgo, blockmode, padding)
                           .toLatin1().constData())) {
         
         // get client's public key
         QCA::DHPublicKey clientKey(dlgroup,
                                    QCA::BigInteger(QCA::SecureArray(input.toByteArray())));
         // generate own private key
         QCA::PrivateKey privKey(keygen.createDH(dlgroup));
         // generate the shared symmetric key
         QCA::SymmetricKey sharedKey(privKey.deriveKey(clientKey));

         QCA::Cipher::Mode cbm;
         if (blockmode == "cbc") {
            cbm = QCA::Cipher::CBC;
         } else {
            return 0;
         }
         
         QCA::Cipher::Padding cp;
         if (padding == "pkcs7") {
            cp = QCA::Cipher::PKCS7;
         } else if (padding == "default") {
            cp = QCA::Cipher::DefaultPadding;
         } else {
            return 0;
         }
         
         QCA::Cipher *cipher = new QCA::Cipher(encalgo, cbm, cp);
         
         // check if creating the cipher worked and if our shared
         // key is longer than the minimum length required.
         if (sharedKey.size() >= cipher->keyLength().minimum()) {
            // generate the response to the client so it can derive
            // the key as well.
            session = new Session(parent);
            session->m_encrypted = true;
            session->m_cipher = cipher;
            session->m_symmetricKey = sharedKey;
            output.setValue(privKey.toPublicKey().toDH().y().toArray().toByteArray());
         } else {
            return 0;
         }
      }
   } else {
      return 0;
   }

   // creating the session was successful
   session->m_peer = peer;
   return session;
}

Secret Session::encrypt(const QCA::SecureArray &value, bool &ok)
{
   ok = false;

   Secret s;
   s.setSession(m_objectPath);
   if (m_encrypted) {
      Q_ASSERT(m_cipher);
      QCA::InitializationVector iv(m_cipher->blockSize());
      m_cipher->setup(QCA::Encode, m_symmetricKey, iv);
      QCA::SecureArray encval = m_cipher->update(value);
      if (!m_cipher->ok()) {
         return s;
      }
      encval += m_cipher->final();
      if (m_cipher->ok()) {
         return s;
      }
      s.setValue(encval.toByteArray());
      s.setParameters(iv.toByteArray());
   } else {
      s.setValue(value.toByteArray());      
   }


   ok = true;
   return s;
}

QCA::SecureArray Session::decrypt(const Secret &secret, bool &ok)
{
   // make sure this is really meant for us
   Q_ASSERT(secret.objectPath() == m_objectPath);
   ok = false;
   
   QCA::SecureArray value;
   if (m_encrypted) {
      Q_ASSERT(m_cipher);
      if (!secret.parameters().size() == m_cipher->blockSize()) {
         return value;
      }
      QCA::InitializationVector iv(secret.parameters());
      m_cipher->setup(QCA::Decode, m_symmetricKey, iv);
      value = m_cipher->update(secret.value());
      if (!m_cipher->ok()) {
         return value;
      }
      value += m_cipher->final();
      if (!m_cipher->ok()) {
         return value;
      }
   } else {
      value = secret.value();
   }

   ok = true;
   return value;
}

void Session::close()
{
   deleteLater();
}

const QString &Session::peer() const
{
   return m_peer;
}

#include "session.moc"
