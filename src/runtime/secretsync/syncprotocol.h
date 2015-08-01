/*
 * Copyright 2010, Valentin Rusu <valir@kde.org>
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
#ifndef SYNCPROTOCOL_H
#define SYNCPROTOCOL_H

#include <QString>
#include <QMetaType>

class SyncLogger;

/**
 * @class SyncProtocol
 * 
 * This class implements the synchronization protocol between the peers.
 * Note that it's designed to work over an already established connection.
 * See the SyncProtocol.odt for the specification of this protocol
 */
class SyncProtocol {
public:
    enum State {
        STATE_INIT,
        STATE_HELLO,
        STATE_LIST_ITEMS,
        STATE_DONE,
        STATE_ERROR,
        STATE_COUNT /// this status is used only to have a maximum index for the status 
    };

    static const int VERSION;
    
    class Phase {
    protected:
        Phase(SyncProtocol* protocol, State state) : _protocol( protocol ), _state( state ) {}
        
    public:
        virtual ~Phase() {};
        
        virtual QString request() = 0;
        virtual State handleRequest( const QString& request, QString& response ) = 0;
        virtual State errorHandlingRequest( const QString& request );
        State handleReply( const QString& reply );
        virtual State handleOkReply( int statusCode, const QString& message ) =0;
        virtual State handleErrorReply( int statusCode, const QString& message );
        virtual void onInvalidReply() {};
        const char* name() const;
    protected:
        SyncProtocol    *_protocol;
        State           _state;
        int             _replyStatusCode;
        QString         _replyMessage;
    };

    class PhaseHello : public Phase {
    public:
        PhaseHello(SyncProtocol *protocol) : Phase( protocol, STATE_HELLO ) {}
        virtual QString request();
        virtual State handleRequest( const QString& request, QString& response );
        virtual State handleOkReply( int statusCode, const QString& message );
    };
    
    class PhaseListItems : public Phase {
        static const char* REQUEST;
        static const char* NO_MORE_ITEMS;
    public:
        PhaseListItems(SyncProtocol *protocol) : Phase( protocol, STATE_LIST_ITEMS ) {}
        virtual QString request();
        virtual State handleRequest( const QString& request, QString& response );
        virtual State handleOkReply( int statusCode, const QString& message );
        
    };
    
    class PhaseDone : public Phase {
    public:
        PhaseDone(SyncProtocol *protocol) : Phase( protocol, STATE_DONE ) {}
        virtual QString request();
        virtual State handleRequest( const QString& request, QString& response );
    };
    
protected:
    SyncProtocol( SyncLogger* logger );
    virtual ~SyncProtocol();

    void changeState( State );
    Phase *phase() const { Q_ASSERT(_phase != 0); return _phase; }
    bool tryHandlingReply( const QString& );
    bool tryHandlingRequest( const QString& request, QString &reply );
    
    void createLogEntry( const QString& );
    bool connectToSecretService();
    void disconnectFromSecretService();
    
public:
    bool isDone() const { return _state == STATE_DONE; }
    QString errorString() const { return _errorString; }
    State state() const { return _state; }
    
private:
    State                       _state;
    Phase                       *_phase;
    QString                     _errorString;
    SyncLogger                  *_logger;
    friend class PhaseListItems;
};

class SyncProtocolServer : public SyncProtocol {
public:
    explicit SyncProtocolServer( SyncLogger *logger );

    /**
     * @param request is the client request
     * @param response reference to a QString which will contain the server response upon successful return of this method
     * @return true if the request was successfully handled and response is set to a valid reply
     */
    virtual bool handleRequest( const QString& request, QString& response );
};

class SyncProtocolClient : public SyncProtocol {
public:
    explicit SyncProtocolClient( SyncLogger *logger );
    
    /**
     * @return the next request that must be sent to the server
     */
    QString nextRequest();
    
    /**
     * @param serverReply string containing the server reply to the nextRequest()
     * @return true if the server reply was correctly processed
     */
    bool handleReply( const QString& serverReply );
};

#endif // SYNCPROTOCOL_H
