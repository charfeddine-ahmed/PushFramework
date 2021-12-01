#include "StdAfx.h"
#include "TCPSocket.h"

#include "TCPSocketImpl.h"


TCPSocket::TCPSocket()
{
    pImpl = new TCPSocketImpl(this);
}

TCPSocket::~TCPSocket()
{
    delete pImpl;
}

bool TCPSocket::connect( const char* hostAddress, unsigned int uPort, string& strError )
{
    return pImpl->connect(hostAddress, uPort, strError);
}

int TCPSocket::sendRequest( ProtocolFramework::OutgoingPacket* pPacket )
{
	return pImpl->sendRequest(pPacket);
}

int TCPSocket::getStatus()
{
    return pImpl->getStatus();
}

void TCPSocket::disconnect( bool waitForSend /*= true */ )
{
    pImpl->disconnect(waitForSend);
}

void TCPSocket::onPerformAutomatedJob()
{
    //
}


void TCPSocket::OnReadyToSend()
{
	//
}

const char* TCPSocket::explainLastError()
{
    return pImpl->explainLastError();
}

void TCPSocket::onReadyToSendRequests()
{
	//
}

time_t TCPSocket::getLastSentDataTime()
{
	return pImpl->lastTimeDataIsSent;
}

bool TCPSocket::initialize( ProtocolFramework::Protocol* pProtocol, ProtocolFramework::MessageFactory* messageFactory )
{
	return pImpl->initialize(pProtocol, messageFactory);
}


void TCPSocket::onMessage( ProtocolFramework::IncomingPacket* pMsg )
{
	//
}


