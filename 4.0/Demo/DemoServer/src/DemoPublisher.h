#pragma once

#if defined(DEMOPUBLISHER)

#if defined(USE_IQFEED)
//#include "DataLstn.h"
class CDataLstn;
#else // USE_IQFEED
#endif // USE_IQFEED


class DemoPublisher
{
public:
	DemoPublisher(void);
	~DemoPublisher(void);

	void start();

	void stop();

	void doJob();

	static DWORD WINAPI threadProc(LPVOID lpVoid);

	bool PushJsonPacket(Json::Value &Val, BROADCASTQUEUE_NAME queueName);

private:
	HANDLE m_hThread;
	DWORD m_dwThreadId;
	HANDLE m_hKillEvent;//kill event.

	//MyServer &theServer;

#if defined(USE_IQFEED)
#if defined(USE_COMLIBRARY)
  ComLibrary m_rfclSupp;	// Registration Free COM Library
  ComLibrary m_rfclLstn;	// Registration Free COM Library
#endif // USE_COMLIBRARY

#if defined(USE_COMLSTN)
	IDataLstn *m_pDataLstn;
#else // USE_COMLSTN
	CDataLstn *m_pDataLstn;
#endif // USE_COMLSTN

	ItsSupplier *m_pDataSupp;
	ItsSupplierEx *m_pDataSuppEx;
	ItsSymbolListSupplier *m_pSymbSupp;
	ItsSymbolListSupplier2 *m_pExchSupp;
#else // USE_IQFEED
#endif // USE_IQFEED
};

extern DemoPublisher theDemoPublisher;

#endif // DEMOPUBLISHER
