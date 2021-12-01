#include "StdAfx.h"
#include "TCPSocketImpl.h"

#include <process.h>


#include "TCPSocket.h"

#include "ScopedLock.h"


#define nSocketBufferSize 8192


#include <stdio.h>
#include <tchar.h>

#include <conio.h>
#include <ctype.h>
#include <mmsystem.h>

TCPSocketImpl::TCPSocketImpl(TCPSocket* pFacade)
	:theProtocolManager(&memoryAllocator)
{
	this->pFacade = pFacade;

    status = TCPSocket::Disconnected;

    hSocket = NULL;
    hSocketEvent = NULL;

    hThread = NULL;

    bSendInProgress = false;

	messageFactory = NULL;
	pProtocol = NULL;

    isPoolInitialized = false;

    ::InitializeCriticalSection(&cs);
}

TCPSocketImpl::~TCPSocketImpl()
{
    //Stop processing thread :
    if (status != TCPSocket::Disconnected)
    {
        disconnect(false);
    }
    ::DeleteCriticalSection(&cs);
}

bool getAddressFromName(LPCSTR name,LPSTR address,int maxLen){

	in_addr         addr;
	hostent FAR *   host;
	int             len;
	char *          pChar;
	int             position;

	//@
	len = (int)strlen(name);
	for(position = len; position >0;position --){
		if(name[position]=='@'){
			position++;
			break;
		}
	}

	host = gethostbyname (&name[position]);

	if(host == NULL)
		return false;

	addr.S_un.S_un_b.s_b1 = host->h_addr_list[0][0];
	addr.S_un.S_un_b.s_b2 = host->h_addr_list[0][1];
	addr.S_un.S_un_b.s_b3 = host->h_addr_list[0][2];
	addr.S_un.S_un_b.s_b4 = host->h_addr_list[0][3];

	pChar = inet_ntoa (addr);

	if(pChar == NULL)
		return false;

	len = (int)strlen(pChar) +1;

	if(len > maxLen)
		return false;

	strcpy_s(address, maxLen, pChar);

	return true;
}

bool TCPSocketImpl::connect( const char* hostAddress, unsigned int uPort, string& strError )
{
    ScopedLock lock(cs);

    if(status != TCPSocket::Disconnected)
        return true;
	
	if (inet_addr(hostAddress) == INADDR_NONE)
	{
		char strIPAddress[30];
		if (!getAddressFromName(hostAddress, strIPAddress, sizeof(strIPAddress))){
			strError = getLastErrorStdStr();
			return false;
		}
		this->strIP = strIPAddress;
	}
	else
		this->strIP = hostAddress;

    this->uPort = uPort;

    hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (hSocket == INVALID_SOCKET)
    {
        return false;
    }


	int windowSize = nSocketBufferSize;

	int nErr = setsockopt(hSocket, SOL_SOCKET, SO_SNDBUF, (char*) &windowSize, sizeof(windowSize));
	if (nErr == -1)
	{
		closesocket(hSocket);
		return false;
	}

	nErr = setsockopt(hSocket, SOL_SOCKET, SO_RCVBUF, (char*) &windowSize, sizeof(windowSize));
	if (nErr == -1)
	{
		closesocket(hSocket);
		return false;
	}

    SOCKADDR_IN		saServer;
    memset(&saServer,0,sizeof(saServer));

    saServer.sin_family = AF_INET;
    saServer.sin_addr.s_addr = inet_addr(strIP.c_str());
    saServer.sin_port = htons(uPort);

    int nRet = ::connect(hSocket,(sockaddr*)&saServer, sizeof(saServer));

    if (nRet == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
    {
		strError = getLastErrorStdStr();
		closesocket(hSocket);
		return false;
    }

    bool m_bInProgress = true;

    hSocketEvent = WSACreateEvent();
    if (hSocketEvent == WSA_INVALID_EVENT)
    {
        closesocket(hSocket);
        return FALSE;
    }

    nRet = WSAEventSelect(hSocket,
                          hSocketEvent,
                          FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE);

    if (nRet == SOCKET_ERROR)
    {
        closesocket(hSocket);
        return false;
    }

    status = TCPSocket::Connecting;

    hKillEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    UINT m_dwThreadId;
    hThread = (HANDLE)_beginthreadex(NULL, 0, networkJobsThreadProc, (void*) this, 0, &m_dwThreadId);

    return true;
}

unsigned __stdcall TCPSocketImpl::networkJobsThreadProc( LPVOID lParam )
{
    TCPSocketImpl* pThis = reinterpret_cast<TCPSocketImpl*>(lParam);

    pThis->doNetworkJobs();

    return 0;
}

bool TCPSocketImpl::OnConnect()
{
    ScopedLock lock(cs);

    status = TCPSocket::Connected;

	//Advance initialization.
	Buffer protocolBytes(&memoryAllocator);
	Protocol* pProtocol = AdvanceInitialization(protocolBytes);
	if (pProtocol)
	{
		sendBufferBytesRequest(protocolBytes, pProtocol->getLowerProtocol());
	}

	CheckConnectionInitialization();

    return true;
}

bool TCPSocketImpl::OnClose()
{
    return false;
}

bool TCPSocketImpl::OnRead()
{
   Buffer incomingBytes(&memoryAllocator, nSocketBufferSize);
   if (!ReadReceivedBytes(incomingBytes))
   {
	   return false;
   }

   if (!theProtocolManager.processIncomingData(*this, incomingBytes))
   {
	   return false;
   }

   DeserializeData deserializeData(&memoryAllocator, getProtocol()->getLowestProtocol());
   NetworkDeserializeResult::Type result;

   while (true)
   {
	   deserializeData.clear();
		result = theProtocolManager.tryDeserializeIncomingPacket(*this, deserializeData);
		
		if (result == NetworkDeserializeResult::ProtocolBytes)
		{	
			if (deserializeData.protocolBytes.hasBytes())
			{
				sendBufferBytesRequest(deserializeData.protocolBytes, deserializeData.pProtocol->getLowerProtocol());
			}

			continue;
		}
		else if (result == NetworkDeserializeResult::Content)
		{
			pFacade->onMessage(deserializeData.pMessage);
			messageFactory->disposeIncomingPacket(deserializeData.pMessage);
		}
		else if(result == NetworkDeserializeResult::WantMoreData)
		{
			break;
		}
		else
			/*result == NetworkDeserializeResult::Failure or 
			NetworkDeserializeResult::Initializationfailure*/
		{
			break;
		}	
   }

   if (result != NetworkDeserializeResult::WantMoreData)
   {
	   return false;
   }

   return true;
}

void TCPSocketImpl::LogStatistics()
{
	//m_stats.Log();
}

int TCPSocketImpl::sendBufferBytesRequest(Buffer& protocolBytes, Protocol* pProtocol)
{
	SerializeData serializeData(pProtocol);
    Buffer encodeResult(&memoryAllocator);
	NetworkSerializeResult::Type result = theProtocolManager.serializeOutgoingBytes(*this, protocolBytes, encodeResult, serializeData);
    oBuffer.addBuffer(encodeResult);

	if (result != NetworkSerializeResult::Success)
	{
		//stats.addToCumul(ServerStats::BandwidthRejection, uWrittenBytes);
		return result==NetworkSerializeResult::Retry ? TCPSocket::ResultTryAgain : TCPSocket::ResultFailed; //TODO. There is the fatal protocol failure case.
	}

	if (WriteBytes())
	{
		return TCPSocket::ResultOK;
	}
	else
	{
		//m_stats.nMessagesOutFailed++;
		return TCPSocket::ResultFailed;
	}
}
int TCPSocketImpl::sendRequest( ProtocolFramework::OutgoingPacket* pPacket )
{
	//m_stats.nMessagesOut ++;messageFactory

	SerializeData serializeData(getProtocol());
    Buffer encodeResult(&memoryAllocator);
    NetworkSerializeResult::Type result = theProtocolManager.serializeOutgoingPacket(*this, *pPacket, encodeResult, serializeData);
    oBuffer.addBuffer(encodeResult);

	if (result != NetworkSerializeResult::Success)
	{
		//stats.addToCumul(ServerStats::BandwidthRejection, uWrittenBytes);
		return result==NetworkSerializeResult::Retry ? TCPSocket::ResultTryAgain : TCPSocket::ResultFailed; //TODO. There is the fatal protocol failure case.
	}

	if (WriteBytes())
	{
		return TCPSocket::ResultOK;
	}
	else
	{
		return TCPSocket::ResultFailed;
	}
}

bool TCPSocketImpl::WriteBytes()
{
    if (oBuffer.isEmpty())
        return true;

    unsigned int uSize = min(nSocketBufferSize, oBuffer.getDataSize());
    
    Buffer ioBuffer(&memoryAllocator, uSize);
    oBuffer.copyTo(ioBuffer, uSize);

    int dwBytes = send(hSocket, ioBuffer.getBuffer(), uSize, 0);

    if (dwBytes == SOCKET_ERROR)
    {
        if (GetLastError() == WSAEWOULDBLOCK)
        {
            bSendInProgress = true;
            return true;
        }
        else
        {
            closesocket(hSocket);
            return false;
        }
    }
    else
    {
		lastTimeDataIsSent = time(NULL);
        oBuffer.pop(dwBytes);
        return WriteBytes();
    }
}

bool TCPSocketImpl::OnWrite()
{
	lastTimeDataIsSent = time(NULL);

	//
    bSendInProgress = false;

    if (oBuffer.isEmpty())
    {
        pFacade->OnReadyToSend();
        return true;
    }

    return WriteBytes();
}

int TCPSocketImpl::getStatus()
{
    return status;
}

void TCPSocketImpl::disconnect( bool waitForSend )
{
	if (status == TCPSocket::Disconnected)
	{
		return; // Nothing to do.
	}

    if (waitForSend && oBuffer.getDataSize()!=0)
    {
        status = TCPSocket::WaitingToClose;
        return;
    }

    closesocket(hSocket);
    //Stop the processing thread.

    ::SetEvent(hKillEvent);

	WaitForSingleObject(hThread, INFINITE);

    status = TCPSocket::Disconnected;
}

const char* TCPSocketImpl::explainLastError()
{
    DWORD dwIOError = GetLastError();

    LPVOID lpMsgBuf;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwIOError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) &lpMsgBuf,
        0, NULL );

    return (LPCSTR)lpMsgBuf;
}

Protocol* TCPSocketImpl::getProtocol()
{
	return pProtocol;
}

HANDLE TCPSocketImpl::CreatePeriodicTimer() 
{
    HANDLE hTimer = NULL;
    LARGE_INTEGER liDueTime;

    const int unitsPerSecond=10*1000*1000; // 100 nano seconds

    // Set the event the first time 1 seconds
    // after calling SetWaitableTimer

    liDueTime.QuadPart=-(1*unitsPerSecond);

    // Create an unnamed waitable timer.
    hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
    if (NULL == hTimer)
    {
        return NULL;
    }

    // Set a timer to wait for 10 seconds.
    if (!SetWaitableTimer(hTimer, &liDueTime, 1000, NULL, NULL, 0))
    {
        return NULL;
    }
    return hTimer;
}
void TCPSocketImpl::doNetworkJobs()
{
    HANDLE hTimer = CreatePeriodicTimer();

    //
    HANDLE			hWaits[3];
    hWaits[0]		= hKillEvent;
    hWaits[1]		= hTimer;
    hWaits[2]		= hSocketEvent;

    DWORD dwRet;

    WSANETWORKEVENTS events;

    while (true)
    {
        dwRet = WSAWaitForMultipleEvents(3, hWaits,	FALSE, INFINITE, FALSE);

        if (dwRet == WAIT_OBJECT_0)
        {
            break;
        }
        
        if (dwRet == WAIT_OBJECT_0 + 1)
        {
            pFacade->onPerformAutomatedJob();
            continue;
        }

        // Figure out what happened
        int nRet = WSAEnumNetworkEvents(hSocket, hSocketEvent, &events);

        if (nRet == SOCKET_ERROR)
        {
            int ierror = WSAGetLastError();
            break;
        }

        //Dispatch event :
        if(events.lNetworkEvents & FD_CONNECT)
        {
            if(!OnConnect())
                break;
        }

        if(events.lNetworkEvents & FD_READ)
        {
            if(!OnRead())
                break;
        }

        if(events.lNetworkEvents & FD_CLOSE)
        {
            if(!OnClose())
                break;
        }


        if(events.lNetworkEvents & FD_WRITE)
        {
            if(!OnWrite())
                break;
        }
    }

    CloseHandle(hKillEvent);

    WSACloseEvent(hSocketEvent);
    status = TCPSocket::Disconnected;

	if (dwRet != WAIT_OBJECT_0)
	{
		pFacade->onConnectionClosed();
	}
    
}

std::string TCPSocketImpl::getLastErrorStdStr()
{
	DWORD error = WSAGetLastError();
	if (error)
	{
		LPVOID lpMsgBuf;
		DWORD bufLen = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR) &lpMsgBuf,
			0, NULL );
		if (bufLen)
		{
			LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
			std::string result(lpMsgStr, lpMsgStr+bufLen);

			LocalFree(lpMsgBuf);

			return result;
		}
	}
	return std::string();
}

MessageFactory& TCPSocketImpl::getMessageFactory()
{
	return *messageFactory;
}

void TCPSocketImpl::InitializeConnection()
{
	pFacade->onConnected();
}

bool TCPSocketImpl::ReadReceivedBytes( Buffer& incomingBytes )
{
	DWORD dwSize = 0;
	int nRet = ioctlsocket(hSocket, FIONREAD, &dwSize);

	if (nRet == -1)
		return true;

	if(dwSize == 0)
		return false;

    int nBytesToBeRead = min(dwSize, nSocketBufferSize);

	int nRead = recv(hSocket, incomingBytes.getBuffer(0), nBytesToBeRead, 0);
	if (nRead == -1)
		return false;

	incomingBytes.setPosition(nRead);

	return true;
}

bool TCPSocketImpl::initialize( ProtocolFramework::Protocol* pProtocol, ProtocolFramework::MessageFactory* messageFactory )
{
#ifdef Plateform_Windows
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

	this->messageFactory = messageFactory;
	this->pProtocol = pProtocol;

	pProtocol->initialize(1, &memoryAllocator);
	this->SetUpProtocolContexts();

	//
	return true;
}


