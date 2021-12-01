#pragma once


class TCPSocket;
class ResponseHandler;

class TCPSocketImpl : public Connection
{
public:
    TCPSocketImpl(TCPSocket* pFacade);
    ~TCPSocketImpl(void);

	bool initialize(ProtocolFramework::Protocol* pProtocol, ProtocolFramework::MessageFactory* messageFactory);

    int getStatus();

    bool connect(const char* hostAddress, unsigned int uPort, string& strError);

    int sendRequest(ProtocolFramework::OutgoingPacket* pPacket);
	int sendBufferBytesRequest(Buffer& protocolBytes, Protocol* pProtocol);
    void disconnect(bool waitForSend);
    const char* explainLastError();

	void LogStatistics();

	

private:
    TCPSocket* pFacade;

    int status;

    std::string strIP;
    unsigned int uPort;

    SOCKET hSocket;

    HANDLE hThread;
    HANDLE hKillEvent;
  
    WSAEVENT hSocketEvent;

    ProtocolFramework::BufferList oBuffer;

    ProtocolFramework::Protocol* pProtocol;
	ProtocolFramework::MessageFactory* messageFactory;

	CRITICAL_SECTION cs;

	ProtocolFramework::ProtocolContext* pContext;

	ProtocolManager theProtocolManager;

	bool isPoolInitialized;
	MemoryAllocator memoryAllocator;

private:

    static unsigned __stdcall networkJobsThreadProc(LPVOID lParam);
    void doNetworkJobs();

    HANDLE CreatePeriodicTimer();

    bool OnRead();
    bool OnWrite();
    bool OnConnect();
    bool OnClose();

    bool bSendInProgress;

    bool WriteBytes();

    bool stopClient;

	std::string getLastErrorStdStr();

	bool ReadReceivedBytes(Buffer& incomingBytes);

	//CStats m_stats;

public:

	time_t lastTimeDataIsSent;

public:
	virtual Protocol* getProtocol();
	virtual MessageFactory& getMessageFactory();
	virtual void InitializeConnection();
};
