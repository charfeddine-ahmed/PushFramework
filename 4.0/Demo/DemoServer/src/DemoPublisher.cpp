#include "StdAfx.h"
#include "DemoPublisher.h"
#include "DemoServer.h"

#if defined(DEMOPUBLISHER)

#if defined(USE_IQFEED)
#include "DataLstn.h"
#include "udfdateT.cpp"
#else // USE_IQFEED
#endif // USE_IQFEED

#define nMaximumMessageSize 100 // The maximum size of an abstract message after being encoded.

DemoPublisher::DemoPublisher(void)
	: m_hThread()
	, m_hKillEvent()
	, m_dwThreadId()
#if defined(USE_IQFEED)
	, m_pDataSupp()
	, m_pDataSuppEx()
	, m_pSymbSupp()
	, m_pExchSupp()
#else // USE_IQFEED
#endif // USE_IQFEED
{
#if 0
#if defined(USE_IQFEED)
	HRESULT hr = S_OK;
	ULONG rc = 0;

#if defined(USE_COMLIBRARY)
	hr = m_rfcl.Load(L"C:\\InvestWare\\MultiCharts64\\Datafeeds\\tsIQDataFeed.dll");
	MLogWrite1(_T("DemoPub> m_rfcl.Load, hr=0x%X\n"), hr);
	hr = SUCCEEDED(hr) ? m_rfcl.CreateInstance(CLSID_DATASUPP, &m_pDataSupp) : hr;
#else // USE_COMLIBRARY
	hr = CoInitialize(NULL);
	MLogWrite1(_T("DemoPub> CoInit, hr=0x%X\n"), hr);
	hr = SUCCEEDED(hr) ? CoCreateInstance(CLSID_DATASUPP, NULL, CLSCTX_INPROC_SERVER, IID_ItsSupplier, (void **)&m_pDataSupp) : hr;
#endif // USE_COMLIBRARY
	rc = m_pDataSupp ? m_pDataSupp->AddRef(), m_pDataSupp->Release() : rc;
	MLogWrite1(_T("DemoPub> m_pDataSupp=0x%IX, hr=0x%X, rc=%d\n"), m_pDataSupp, hr, rc);
	hr = m_pDataSupp ? m_pDataSupp->QueryInterface(IID_ItsSupplierEx, (void**)&m_pDataSuppEx) : hr;
	rc = m_pDataSuppEx ? m_pDataSuppEx->AddRef(), m_pDataSuppEx->Release() : rc;
	MLogWrite1(_T("DemoPub> m_pDataSuppEx=0x%IX, hr=0x%X, rc=%d\n"), m_pDataSuppEx, hr, rc);

#if defined(USE_COMLSTN)
#if defined(USE_COMLIBRARY)
	hr = m_rfcl.Load(L"C:\\InvestWare\\MultiCharts64\\InveFeed\\DataLstn.dll");
	hr = SUCCEEDED(hr) ? m_rfcl.CreateInstance(CLSID_DATALSTN, (void **)&m_pDataLstn) : hr;
#else // USE_COMLIBRARY
	hr = CoCreateInstance(CLSID_DATALSTN, NULL, CLSCTX_INPROC_SERVER, IID_IDataLstn, (void **)&m_pDataLstn);
#endif // USE_COMLIBRARY
	rc = m_pDataLstn ? m_pDataLstn->AddRef(), m_pDataLstn->Release() : rc;
	MLogWrite1(_T("DemoPub> m_pDataLstn=0x%IX, hr=0x%X, rc=%d\n"), m_pDataLstn, hr, rc);
#else // USE_COMLSTN
	m_pDataLstn = new CDataLstn(m_pDataSupp);
#endif // USE_COMLSTN
#else // USE_IQFEED
#endif // USE_IQFEED
#endif
}

DemoPublisher::~DemoPublisher(void)
{
#if 0
#if defined(USE_IQFEED)
	HRESULT hr = S_OK;
	ULONG rc = 0;

	rc = m_pDataLstn ? m_pDataLstn->Release() : rc;
	MLogWrite1(_T("~DemoPub> m_pDataLstn=0x%IX=>NULL, rc=%d\n"), m_pDataLstn, rc);
	m_pDataLstn = NULL;

	rc = m_pDataSuppEx ? m_pDataSuppEx->Release() : rc;
	MLogWrite1(_T("~DemoPub> m_pDataSuppEx=0x%IX=>NULL, rc=%d\n"), m_pDataSuppEx, rc);
	m_pDataSuppEx = NULL;

	rc = m_pDataSupp ? m_pDataSupp->Release() : rc;
	MLogWrite1(_T("~DemoPub> m_pDataSupp=0x%IX=>NULL, rc=%d\n"), m_pDataSupp, rc);
	m_pDataSupp = NULL;

#if defined(USE_COMLIBRARY)
	hr = m_rfcl.Unload();
	MLogWrite1(_T("DemoPub> m_rfcl.Unload(F), hr=0x%X\n"), hr);
#else // USE_COMLIBRARY
	CoUninitialize();
	MLogWrite1(_T("DemoPub> CoUninit\n"));
#endif // USE_COMLIBRARY

#else // USE_IQFEED
#endif // USE_IQFEED
#endif
}

void DemoPublisher::start()
{
	m_hKillEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);
	m_dwThreadId = 0;
	m_hThread = CreateThread(NULL, // Security
		0,					// default stack size
		threadProc,  // thread proc
		(void*) this,
		0,					// init flag
		&m_dwThreadId);
	//MLogWrite(_T("DemoPub::start> m_hKillEvent=0x%IX, m_dwThreadId=0x%X, m_hThread=0x%IX\n"), m_hKillEvent, m_dwThreadId, m_hThread);
}

void DemoPublisher::stop()
{
	//MLogWrite(_T("DemoPub::stop> m_hKillEvent=0x%IX=>NULL, m_dwThreadId=0x%X=>0, m_hThread=0x%IX=>NULL\n"), m_hKillEvent, m_dwThreadId, m_hThread);
	if (m_hKillEvent)
	{
		::SetEvent(m_hKillEvent);
		m_hKillEvent = NULL;
	}
	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	m_dwThreadId = 0;
}

DWORD WINAPI DemoPublisher::threadProc( LPVOID lpVoid )
{
	DemoPublisher *pMe = (DemoPublisher *) lpVoid;
	pMe->doJob();
	return 0;
}

// We should not exceed the nMaximumMessageSize size for a particular message.
// 100 is there to make room for the WS header bytes.

string gen_random() {
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	string s;

	for (int i = 0; i < (nMaximumMessageSize); ++i) {
		s+= alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	return s;
}

unsigned int hash( const char *str )
{
	unsigned int hash = 0;
	int c;

	while (c = *str++)
		hash += c;

	return hash;
}

//extern MyServer theServer;
void DemoPublisher::doJob()
{
#if defined(USE_IQFEED)
	HRESULT hr = S_OK;
	ULONG rc = 0;

#if defined(USE_COMLIBRARY)
	hr = m_rfclSupp.Load(L"C:\\InvestWare\\MultiCharts64\\Datafeeds\\tsIQDataFeed.dll");
	MLogWrite1(_T("DemoPub::doJob> m_rfclSupp.Load(), hr=0x%X\n"), hr);
	hr = SUCCEEDED(hr) ? m_rfclSupp.CreateInstance(CLSID_DATASUPP, &m_pDataSupp) : hr;
#else // USE_COMLIBRARY
	hr = CoInitialize(NULL);
	MLogWrite1(_T("DemoPub::doJob> CoInit, hr=0x%X\n"), hr);
	hr = SUCCEEDED(hr) ? CoCreateInstance(CLSID_DATASUPP, NULL, CLSCTX_INPROC_SERVER, IID_ItsSupplier, (void **)&m_pDataSupp) : hr;
#endif // USE_COMLIBRARY
	rc = m_pDataSupp ? m_pDataSupp->AddRef(), m_pDataSupp->Release() : rc;
	MLogWrite1(_T("DemoPub::doJob> m_pDataSupp=0x%IX, hr=0x%X, rc=%d\n"), m_pDataSupp, hr, rc);
	hr = m_pDataSupp ? m_pDataSupp->QueryInterface(IID_ItsSupplierEx, (void**)&m_pDataSuppEx) : hr;
	rc = m_pDataSuppEx ? m_pDataSuppEx->AddRef(), m_pDataSuppEx->Release() : rc;
	MLogWrite1(_T("DemoPub::doJob> m_pDataSuppEx=0x%IX, hr=0x%X, rc=%d\n"), m_pDataSuppEx, hr, rc);

#if defined(USE_COMLSTN)
#if defined(USE_COMLIBRARY)
	hr = m_rfclLstn.Load(L"C:\\InvestWare\\MultiCharts64\\InveFeed\\InvetsLstn.dll");
	MLogWrite1(_T("DemoPub::doJob> m_rfclLstn.Load(), hr=0x%X\n"), hr);
	hr = SUCCEEDED(hr) ? m_rfclLstn.CreateInstance(CLSID_DATALSTN, &m_pDataLstn) : hr;
#else // USE_COMLIBRARY
	hr = CoCreateInstance(CLSID_DATALSTN, NULL, CLSCTX_INPROC_SERVER, IID_IDataLstn, (void **)&m_pDataLstn);
#endif // USE_COMLIBRARY
	rc = m_pDataLstn ? m_pDataLstn->AddRef(), m_pDataLstn->Release() : rc;
	MLogWrite1(_T("DemoPub::doJob> m_pDataLstn=0x%IX, hr=0x%X, rc=%d\n"), m_pDataLstn, hr, rc);
#else // USE_COMLSTN
	m_pDataLstn = new CDataLstn(m_pDataSupp);
	MLogWrite1(_T("DemoPub::doJob> m_pDataLstn=0x%IX\n"), m_pDataLstn);
#endif // USE_COMLSTN
	BSTR name;
	hr = m_pDataSupp->get_Name(&name);
	MLogWrite1(_T("DemoPub::doJob> get_Name=%ls, hr=0x%X\n"), name, hr);
	SysFreeString(name);

	BSTR version;
	hr = m_pDataSupp->get_Version(&version);
	MLogWrite1(_T("DemoPub::doJob> get_Version=%ls, hr=0x%X\n"), version, hr);
	SysFreeString(version);

	BSTR vendor;
	hr = m_pDataSupp->get_Vendor(&vendor);
	MLogWrite1(_T("DemoPub::doJob> get_Vendor=%ls, hr=0x%X\n"), vendor, hr);
	SysFreeString(vendor);

	BSTR desc;
	hr = m_pDataSupp->get_Description(&desc);
	MLogWrite1(_T("DemoPub::doJob> get_Desc=%ls, hr=0x%X\n"), desc, hr);
	SysFreeString(desc);

	hr = m_pDataSupp->Advise(m_pDataLstn);
	MLogWrite1(_T("DemoPub::doJob> Advise(DATALSTN), hr=0x%X\n"), hr);

	LONG Exchange = 150;
	BSTR ExchangeName = L"CME";
	BSTR Sessions = L"5:1980,2430,0,0,1;3420,3870,0,0,1;4860,5310,0,0,1;6300,6750,0,0,1;7740,8190,0,0,1;";
	BSTR SessionTZI = L"-540,0,0,0,0,00:00:00.0000,-60,0,0,0,00:00:00.0000";
	BSTR ExchangeTZI = L"-540,0,0,0,0,00:00:00.0000,-60,0,0,0,00:00:00.0000";
	BSTR Description = L"Description";
	LONG Id = 1;
	LONG Datafeed = 5001;	// iqfeed
	LONG Symbol = 2;
	BSTR SymbolName = L"@ES#";

	//hr = m_pDataSupp->AdviseTransaction(Id, Datafeed, Symbol, SymbolName, Description,
	//		TS_CATEGORY_FUTURE, TS_RESOLUTION_TICK, 1, TS_TRADE_FIELD,
	//		Exchange, ExchangeName,
	//		udfSDT.GetDate(), udfSDT.GetTime(), udfFDT.GetDate(), udfFDT.GetTime(),
	//		Sessions, SessionTZI, ExchangeTZI);
#else // USE_IQFEED
#endif // USE_IQFEED

	unsigned int counter = 0;
#if defined(PUBLISH_QUEUE2)
	unsigned int counter2 = 0;
#endif // PUBLISH_QUEUE2
	while (true)
	{
#if defined(TEST_PINGPUBLISHER)
		DWORD dwMilli = DEMOPUBLISHER_TIMER_PERIOD;
		if (counter > 10)
		{
			dwMilli = 10000;
			theServer.getOptions().uMaxClientIdleTime = 5;
		}
		DWORD dwRet = WaitForSingleObject(m_hKillEvent, dwMilli);
#else // TEST_PINGPUBLISHER
		DWORD dwRet = WaitForSingleObject(m_hKillEvent, DEMOPUBLISHER_TIMER_PERIOD);
#endif // TEST_PINGPUBLISHER

		//If kill event then quit loop :
		if (dwRet != WAIT_TIMEOUT)
		{
			//MLogWrite(_T("DemoPub::doJob> WaitFor(m_hKillEvent)=%d\n"), dwRet);
			break;
		}

		//
#if defined(PUBLISH_QUEUE1)
#if defined(DUMMYSERIALIZER)
		DummyMessage *pResp = new DummyMessage(200);
#elif defined(XMLSERIALIZER)
		XMLMessage *pResp = new XMLMessage(1);
		string text = ::gen_random();
		pResp->setArgumentAsText("text", text.c_str());
		pResp->setArgumentAsInt("hash", ::hash(text.c_str()));
		pResp->setArgumentAsInt("id", ++counter);
#else
		//Json::Value Resp;	// Will crash!
		Json::Value *pResp = new Json::Value();
		Json::Value &Resp = *pResp;
		Resp["routingId"] = Demo::Publish;//Broadcast;
		Resp["queueName"] = "queue1";
		string text = ::gen_random();
		Resp["message"] = text;
		Resp["hash"] = ::hash(text.c_str());
		Resp["id"] = ++counter;
#endif // DUMMYSERIALIZER

		//PushJsonPacket(response, "queue1");
		OutgoingPacket *outgoingPacket = reinterpret_cast<OutgoingPacket *> (pResp);
		theServer.PushPacket(outgoingPacket, "queue1");
#endif // PUBLISH_QUEUE1

#if defined(PUBLISH_QUEUE2)
#if defined(DUMMYSERIALIZER)
		DummyMessage *pResp2 = new DummyMessage(200);
#elif defined(XMLSERIALIZER)
		XMLMessage *pResp2 = new XMLMessage(1);
		string text2 = ::gen_random();
		pResp2->setArgumentAsText("text", text2.c_str());
		pResp2->setArgumentAsInt("hash", ::hash(text2.c_str()));
		pResp2->setArgumentAsInt("id", ++counter2);
#else
		//Json::Value Resp2;	// Will crash!
		Json::Value *pResp2 = new Json::Value();
		Json::Value &Resp2 = *pResp2;
		Resp2["routingId"] = Demo::Publish;//Broadcast;
		Resp2["queueName"] = "queue2";
		string text2 = ::gen_random();
		Resp2["message"] = text2;
		Resp2["hash"] = ::hash(text2.c_str());
		Resp2["id"] = ++counter2;
#endif // DUMMYSERIALIZER

		//PushJsonPacket(Resp2, "queue2");
		OutgoingPacket *outgoingPacket2 = reinterpret_cast<OutgoingPacket *> (pResp2);
		theServer.PushPacket(outgoingPacket2, "queue2");
#endif // PUBLISH_QUEUE2
	}

#if defined(USE_IQFEED)
	hr = m_pDataSupp->Unadvise();
	MLogWrite1(_T("DemoPub::doJob> Unadvise(), hr=0x%X\n"), hr);

	rc = m_pDataLstn ? m_pDataLstn->Release() : rc;
	MLogWrite1(_T("DemoPub::doJob> m_pDataLstn=0x%IX=>NULL, rc=%d\n"), m_pDataLstn, rc);
	m_pDataLstn = NULL;

	rc = m_pDataSuppEx ? m_pDataSuppEx->Release() : rc;
	MLogWrite1(_T("DemoPub::doJob> m_pDataSuppEx=0x%IX=>NULL, rc=%d\n"), m_pDataSuppEx, rc);
	m_pDataSuppEx = NULL;

	rc = m_pDataSupp ? m_pDataSupp->Release() : rc;
	MLogWrite1(_T("DemoPub::doJob> m_pDataSupp=0x%IX=>NULL, rc=%d\n"), m_pDataSupp, rc);
	m_pDataSupp = NULL;

#if defined(USE_COMLIBRARY)
	hr = m_rfclLstn.Unload();
	MLogWrite1(_T("DemoPub::doJob> m_rfclLstn.Unload(F), hr=0x%X\n"), hr);
	hr = m_rfclSupp.Unload();
	MLogWrite1(_T("DemoPub::doJob> m_rfclSupp.Unload(F), hr=0x%X\n"), hr);
#else // USE_COMLIBRARY
	CoUninitialize();
	MLogWrite1(_T("DemoPub::doJob> CoUninit\n"));
#endif // USE_COMLIBRARY
#else // USE_IQFEED
#endif // USE_IQFEED
}

DemoPublisher theDemoPublisher;

bool DemoPublisher::PushJsonPacket(Json::Value &Val, BROADCASTQUEUE_NAME queueName)
{
	OutgoingPacket *outgoingPacket = reinterpret_cast<OutgoingPacket *> (&Val);
	return theServer.PushPacket(outgoingPacket, queueName);
}

#endif // DEMOPUBLISHER