/********************************************************************
	File :			ServerImpl.cpp
	Creation date :	2010/6/27

	License :			Copyright 2010 Ahmed Charfeddine, http://www.pushframework.com

				   Licensed under the Apache License, Version 2.0 (the "License");
				   you may not use this file except in compliance with the License.
				   You may obtain a copy of the License at

					   http://www.apache.org/licenses/LICENSE-2.0

				   Unless required by applicable law or agreed to in writing, software
				   distributed under the License is distributed on an "AS IS" BASIS,
				   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
				   See the License for the specific language governing permissions and
				   limitations under the License.


*********************************************************************/
#include "StdAfx.h"
#include "ServerImpl.h"

#include "../include/Server.h"
#include "../include/ServerOptions.h"
#include "LogicalConnectionImpl.h"
#include "LogicalConnectionPool.h"

#ifdef Plateform_Windows
#include "IOQueue_Win.h"
#else
#ifdef Plateform_Linux
#include "IOQueue_Linux.h"
#endif
#endif


#ifdef Plateform_Windows
#include "DemuxImpl_Win.h"
#else
#ifdef Plateform_Linux
#include "DemuxImpl_Linux.h"
#endif
#endif


#include "Dispatcher.h"
#include "ServerStats.h"
#include "Listener.h"
#include "ClientFactory.h"
#include "BroadcastManager.h"
#include "GarbageCollector.h"
#include "BroadcastStreamer.h"
#include "PhysicalConnectionPool.h"
#include "ConnectionContextPool.h"
#include "XMLProtocol.h"
#include "MonitorServer.h"

#ifdef Plateform_Windows
#include <mmsystem.h>
#endif
#include "Utilities.h"


namespace PushFramework
{

#ifdef Plateform_Windows
void ServerImpl::eternal_loop()
{
     pFacade->OnStarted();

    //Periodic Timer
    m_hEventTimer = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(m_hEventTimer);

    //Abort event. (set by stop function)
    m_hEventAbort = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(m_hEventAbort);
    //

    HANDLE			hWaits[2];
    hWaits[0]		= m_hEventTimer;
    hWaits[1]		= m_hEventAbort;

    UINT			sleepTimeMs = 1000; //1 second.

    nSecsGCCount=0;
    nSecsPerfObservCount=0;
    //

    while(true)
    {
        MMRESULT result = timeSetEvent(sleepTimeMs, 1,
                                       (LPTIMECALLBACK) m_hEventTimer, 0, TIME_ONESHOT|TIME_CALLBACK_EVENT_SET);
        _ASSERT(result != NULL);

        // Sleep here until the timer interval ends or abort occurs
        if(WaitForMultipleObjects(2, hWaits, FALSE, INFINITE)==0)
        {

            DoPeriodicJobs();
            ResetEvent(m_hEventTimer);
        }
        else
        {
            //m_hEventAbort is set : break from while.
            break;
        }
        //
    }

    CloseHandle(m_hEventAbort);
    CloseHandle(m_hEventTimer);
    //quit blocking.
}

DWORD WINAPI ServerImpl::threadProc( LPVOID WorkContext )
{
    ServerImpl* pMe = reinterpret_cast<ServerImpl*>(WorkContext);
    pMe->eternal_loop();
    return 0;
}
#endif


#ifdef Plateform_Linux
void ServerImpl::eternal_loop()
{
    pFacade->OnStarted();

    int sec = 1;
    struct timespec ts;
    pthread_mutex_t cs;
    pthread_mutex_init(&cs, NULL);
    pthread_mutex_lock(&cs);
    pthread_cond_init(&abortCond, NULL);

    nSecsGCCount = 0;
    nSecsPerfObservCount = 0;

    while (true)
    {
        ::clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += sec;

        int rc = pthread_cond_timedwait(&abortCond, &cs, &ts);
        if (rc == ETIMEDOUT)
        {
           DoPeriodicJobs();
        }
        else
        {
            //Server is to die.
            break;
        }

    }


    pthread_mutex_unlock(&cs);

}
void* ServerImpl::threadProc(void* WorkContext)
{
    ServerImpl* pMe = reinterpret_cast<ServerImpl*> (WorkContext);
    pMe->eternal_loop();
    return 0;
}
#endif


bool ServerImpl::start( bool startInSeparateThread )
{
	if (pServerOptions->isMonitorEnabled)
	{
		pMonitorServer = new MonitorServer(getServerStats());

		if (!pMonitorServer->configure(pServerOptions->monitorPort, pServerOptions->password))
		{
			delete pMonitorServer;
			return false;
		}
	}


	unsigned int nMaxPoolConnections = pServerOptions->nMaxConnections + 10;
	unsigned int nMaxThreads =  pServerOptions->nWorkersCount + 20;

	//TODO. Remove.
	/*unsigned int nMessageSize = calculateMaxBufferSizePerMessage();
	unsigned int nMaxAdditionalMsgBuffersForProtocols = calculateAdditionalBuffersForProtocols(nMaxPoolConnections, nMaxThreads);
	if (!pMsgBufferPool->create(RecyclableBuffer::Single, nMaxThreads*4 + nMaxAdditionalMsgBuffersForProtocols, nMessageSize))
		return false;

	if(!pMsgBufferPool->create(RecyclableBuffer::Double, nMaxPoolConnections, nMessageSize*2))
		return false;

	if(!pMsgBufferPool->create(RecyclableBuffer::Multiple, nMaxPoolConnections, pServerOptions->nMaxPendingOutgoingMessages*nMessageSize))
		return false;
	
	if(!pMsgBufferPool->create(RecyclableBuffer::Socket, nMaxPoolConnections*2, pServerOptions->nSocketBufferSize))
		return false;*/

	if (!pPhysicalConnectionPool->initialize(nMaxPoolConnections))
		return false;

	if (!pLogicalConnectionPool->initialize(nMaxPoolConnections))
		return false;

	if (pServerOptions->challengeClients)
	{
		if(!pConnectionContextPool->initialize(nMaxPoolConnections))
			return false;
	}

	if(!initializeProtocolContextPools(nMaxPoolConnections))
		return false;

    if (!pIOQueue->create())
        return false;

    if (!pDemux->start())
    {
        pIOQueue->free();
        return false;
    }

    if (!StartListening())
    {
        pDemux->stop();
        pIOQueue->free();
		return false;
    }

    startStreamers();

    if (startInSeparateThread)
    {
#ifdef Plateform_Windows
        DWORD  nThreadID;

        hThread = CreateThread(NULL,// Security
                                         0,						// Stack size - use default
                                         threadProc,     		// Thread fn entry point
                                         (void*) this,			// Param for thread
                                         0,						// Init flag
                                         &nThreadID);			// Thread address
#else
#ifdef Plateform_Linux
        pthread_create(&hThread, NULL, threadProc, (void*) this);
#endif
#endif
    }
    else
        eternal_loop();

	return true;
}

void ServerImpl::stop()
{
#ifdef Plateform_Windows
    SetEvent(m_hEventAbort);//stops eternal_loop
    WaitForSingleObject(hThread, INFINITE);
#else
#ifdef Plateform_Linux
    pthread_cond_signal(&abortCond);
#endif
#endif

    StopListening();

    pDemux->stop();

    pClientFactory->stop();

	stopStreamers();

    pBroadcastMgr->disposeAllPackets();

    pIOQueue->free();

    pGarbageCollector->activate(true);

	//Free memory pools:
	pPhysicalConnectionPool->deleteAllocatedMemory();
	pLogicalConnectionPool->deleteAllocatedMemory();
	pConnectionContextPool->deleteAllocatedMemory();
	uninitializeProtocolContextPools();

	if (pServerOptions->isMonitorEnabled)
	{
		pMonitorServer->stop();
	}

    DestroyListeners();
}

void ServerImpl::registerService(Service* pService)
{
    pDispatcher->registerService(pService);
}

ServerImpl::ServerImpl( Server* pFacade )
{
#ifdef Plateform_Windows
    //Initialize WinSock :
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    this->pFacade = pFacade;

    uGCPeriod = 10;

    nSecsGCCount=0;
    nSecsPerfObservCount=0;

	pMsgMemAllocator = new MemoryAllocator();
	pIOQueue = new IOQueue();
	pDemux = new DemuxImpl(this);
	pClientFactory = new ClientFactory(this);
	pDispatcher = new Dispatcher(this);
	pGarbageCollector = new GarbageCollector(this);
	pProtocolMgr = new ProtocolManager(pMsgMemAllocator);
	pBroadcastMgr = new BroadcastManager(this);
	pServerStats = new ServerStats(this);

	//
	pLogicalConnectionPool = new LogicalConnectionPool(this);
	pConnectionContextPool = new ConnectionContextPool(this);
	pPhysicalConnectionPool = new PhysicalConnectionPool(this);
	pServerOptions = new ServerOptions();
	pMonitorServer = NULL;
	
}

ServerImpl::~ServerImpl( void )
{
	delete pIOQueue;
	delete pDemux;
	delete pClientFactory;
	delete pDispatcher;
	delete pGarbageCollector;
	delete pProtocolMgr;
	delete pBroadcastMgr;
	delete pServerStats;
	//
	delete pLogicalConnectionPool;
	delete pConnectionContextPool;
	delete pPhysicalConnectionPool;
	delete pServerOptions;
	delete pMsgMemAllocator;

	if (pMonitorServer)
		delete pMonitorServer;
}

Server* ServerImpl::getFacade()
{
    return pFacade;
}

void ServerImpl::setServeInfos( const char* serverInfos )
{
    this->serverInfos = serverInfos;
}

std::string ServerImpl::getServerInfos()
{
    return serverInfos;
}

void ServerImpl::pause()
{
	StopListening();
}

void ServerImpl::resume()
{
	StartListening();
}

bool ServerImpl::createListener( int port, ListenerOptions* pOptions /*= NULL*/ )
{
	listenersMapT::iterator it = listenersMap.find(port);
	if (it != listenersMap.end())
	{
		return false;
	}

	Listener* pListener = new Listener(this);
	pListener->setListeningPort(port);
	pListener->setOptions(*pOptions);

	listenersMap[port] = pListener;
    return true;
}

void ServerImpl::DoPeriodicJobs()
{
    nSecsGCCount++;
    nSecsPerfObservCount++;

    if(nSecsGCCount == uGCPeriod)
    {
        nSecsGCCount = 0;
        pGarbageCollector->activate();
        pClientFactory->scrutinize();
    }

    if(nSecsPerfObservCount == pServerOptions->samplingRate && pServerOptions->isProfilingEnabled)
    {
        nSecsPerfObservCount=0;
        OutgoingPacket* pPacket = pServerStats->getPerformancePacket();
		pMonitorServer->PushPacket(pPacket, "stats");
    }
}

void ServerImpl::setMessageFactory( MessageFactory* pMsgFactory )
{
	this->pMsgFactory = pMsgFactory;
}

MessageFactory& ServerImpl::getMessageFactory()
{
	return *pMsgFactory;
}

void ServerImpl::startStreamers()
{
	for (unsigned int i=0; i<pServerOptions->nStreamers; i++)
	{
		BroadcastStreamer* pStreamer = new BroadcastStreamer();
		streamersList.push_back( pStreamer);
		pStreamer->start();
	}
}

void ServerImpl::stopStreamers()
{
	streamersListT::iterator it = streamersList.begin();
	while (it!=streamersList.end())
	{
		BroadcastStreamer* pStreamer = *it;
		pStreamer->stop();
		delete pStreamer;
		it = streamersList.erase(it);
	}
}

void ServerImpl::reshuffleStreamers()
{
	streamersListT::iterator it = streamersList.begin();
	while (it!=streamersList.end())
	{
		BroadcastStreamer* pStreamer = *it;
		pStreamer->awakeAll();
		it++;
	}
}

void ServerImpl::addClientToStreamers( LogicalConnectionImpl* pLogicalConnection )
{
	vector<string>& globalQueues = pBroadcastMgr->getGlobalQueues();

	for (unsigned int i=0;i<globalQueues.size();i++)
	{
		pLogicalConnection->SubscribeToQueue(globalQueues[i].c_str());
	}

	BroadcastStreamer* pLessBusy = NULL;

	streamersListT::iterator it = streamersList.begin();
	while (it!=streamersList.end())
	{
		if (pLessBusy)
		{
			if (pLessBusy->getItemsCount() > (*it)->getItemsCount())
			{
				pLessBusy = (*it);
			}
		}
		else
			pLessBusy = (*it);
		it++;
	}

	if (pLessBusy)
	{
		pLessBusy->addItem(pLogicalConnection);
		pLogicalConnection->pStreamer = pLessBusy;
	}
}

void ServerImpl::CreateQueue( BROADCASTQUEUE_NAME queueName, QueueOptions& queueOptions )
{
	pBroadcastMgr->createBroadcastQueue(queueName, queueOptions);
}

void ServerImpl::RemoveQueue( BROADCASTQUEUE_NAME queueName )
{
	pBroadcastMgr->removeBroadcastQueue(queueName);
}

bool ServerImpl::PushPacket( OutgoingPacket* pPacket, BROADCASTQUEUE_NAME queueName )
{
	return pBroadcastMgr->pushPacket(pPacket, queueName, "", 0);
}

bool ServerImpl::PushPacket( OutgoingPacket* pPacket, BROADCASTQUEUE_NAME queueName, BROADCASTPACKET_KEY killKey, int objectCategory )
{
	return pBroadcastMgr->pushPacket(pPacket, queueName, killKey, objectCategory);
}

void ServerImpl::RemovePacketFromQueue( BROADCASTPACKET_KEY killKey, int objectCategory, BROADCASTQUEUE_NAME queueName )
{
	pBroadcastMgr->removePacket(killKey, objectCategory, queueName);
}

void ServerImpl::StopListening()
{
	listenersMapT::iterator it = listenersMap.begin();
	while (it != listenersMap.end())
	{
		Listener* pListener = it->second;
		pListener->stopListening();
		it++;
	}
}

bool ServerImpl::StartListening()
{
	listenersMapT::iterator it = listenersMap.begin();
	while (it != listenersMap.end())
	{
		Listener* pListener = it->second;
		if (!pListener->startListening())
		{
			StopListening();
			return false;
		}
		it++;
	}
	return true;
}

bool ServerImpl::initializeProtocolContextPools( unsigned int nMaxPoolConnections )
{
	listenersMapT::iterator it = listenersMap.begin();
	while (it != listenersMap.end())
	{
		Listener* pListener = it->second;
		Protocol* pProtocol = pListener->getProtocol();
		while (pProtocol)
		{
			if (!pProtocol->initialize(nMaxPoolConnections, getAllocator()))
				return false;
			pProtocol = pProtocol->getLowerProtocol();
		}

		it++;
	}

	return true;
}

void ServerImpl::uninitializeProtocolContextPools()
{
	listenersMapT::iterator it = listenersMap.begin();
	while (it != listenersMap.end())
	{
		Listener* pListener = it->second;
		Protocol* pProtocol = pListener->getProtocol();
		while (pProtocol)
		{
			pProtocol->deleteAllocatedMemory();
			pProtocol = pProtocol->getLowerProtocol();
		}
		it++;
	}
}

std::string ServerImpl::getListeners()
{
	std::stringstream ss;
	ss << std::noskipws;

	listenersMapT::iterator it = listenersMap.begin();
	while (it != listenersMap.end())
	{
		Listener* pListener = it->second;
		
		ss << "<listener>";
		ss << "<port val=\"" << pListener->getPort() << "\"/>";
		ss << "<protocols>";
		Protocol* pProtocol = pListener->getProtocol();
		while (pProtocol)
		{
			ss << "<protocol>";
			ss << "<name val=\"" << pProtocol->getName() << "\"/>";
			ss << "</protocol>";

			pProtocol = pProtocol->getLowerProtocol();
		}
		ss << "</protocols>";
		ss << "</listener>";

		it++;
	}

	return ss.str();
}

IOQueue& ServerImpl::getIOQueue()
{
	return *pIOQueue;
}

DemuxImpl& ServerImpl::getDemux()
{
	return *pDemux;
}

ClientFactory& ServerImpl::getClientFactory()
{
	return *pClientFactory;
}

Dispatcher& ServerImpl::getDispatcher()
{
	return *pDispatcher;
}

GarbageCollector& ServerImpl::getGarbageCollector()
{
	return *pGarbageCollector;
}

ProtocolManager& ServerImpl::getProtocolMgr()
{
	return *pProtocolMgr;
}

BroadcastManager& ServerImpl::getBroadcastManager()
{
	return *pBroadcastMgr;
}

ServerStats& ServerImpl::getServerStats()
{
	return *pServerStats;
}

LogicalConnectionPool& ServerImpl::getLogicalConnectionPool()
{
	return *pLogicalConnectionPool;
}

ConnectionContextPool& ServerImpl::getConnectionContextPool()
{
	return *pConnectionContextPool;
}

PhysicalConnectionPool& ServerImpl::getPhysicalConnectionPool()
{
	return *pPhysicalConnectionPool;
}

ServerOptions& ServerImpl::getServerOptions()
{
	return *pServerOptions;
}

MemoryAllocator* ServerImpl::getAllocator()
{
	return pMsgMemAllocator;
}

MonitorServer& ServerImpl::getMonitorServer()
{
	return *pMonitorServer;
}

void ServerImpl::DestroyListeners()
{
    listenersMapT::iterator it = listenersMap.begin();
    while (it != listenersMap.end())
    {
        Listener* pListener = it->second;
        delete pListener;
        it++;
    }
}


}

