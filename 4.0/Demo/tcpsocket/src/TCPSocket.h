#pragma once

#include "Symbols.h"


class TCPSocketImpl;

namespace ProtocolFramework
{
class Protocol;
}

class TCPSOCKET_API TCPSocket
{
    friend class TCPSocketImpl;
public:
    enum
    {
        Disconnected = 1,
        Connecting,
        Connected,
        WaitingToClose
    };
	enum
	{
		ResultOK = 0,
		ResultTryAgain,
		ResultFailed,
	};

    TCPSocket();
    ~TCPSocket();

    bool connect(const char* hostAddress, unsigned int uPort, string& strError);
    bool initialize(ProtocolFramework::Protocol* pProtocol, ProtocolFramework::MessageFactory* messageFactory);
    int sendRequest(ProtocolFramework::OutgoingPacket* pPacket);
    int getStatus();
    void disconnect(bool waitForSend = true );
    virtual void OnReadyToSend();
    const char* explainLastError();
	time_t getLastSentDataTime();

protected:
    virtual void onConnected() = 0;
    virtual void onConnectionClosed() = 0;
    virtual void onPerformAutomatedJob();
	virtual void onReadyToSendRequests();
	virtual void onMessage(ProtocolFramework::IncomingPacket* pMsg);

private:
    TCPSocketImpl* pImpl;
};