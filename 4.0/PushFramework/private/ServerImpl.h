/********************************************************************
	File :			ServerImpl.h
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
#ifndef ServerImpl__INCLUDED
#define ServerImpl__INCLUDED

#pragma once




namespace PushFramework
{

///
class ClientFactory;
class Service;
class BroadcastStreamer;
class MonitorProtocol;
class DemuxImpl;
class Dispatcher;
class ServerStats;
class ChannelFactory;
class IOQueue;
class BroadcastManager;
class ClientFactory;
class Listener;
class Server;
class GarbageCollector;
struct Version;
struct ListenerOptions;
struct QueueOptions;
class LogicalConnectionImpl;
class ServerOptions;

class MonitorServer;

class LogicalConnectionPool;
class ConnectionContextPool;
class PhysicalConnectionPool;



class ServerImpl
{
public:
    ServerImpl(Server* pFacade);
    ~ServerImpl(void);

    void setServeInfos(const char* serverInfos);

	void setMessageFactory( MessageFactory* pMsgFactory );
    bool createListener( int port, ListenerOptions* pOptions );
    void registerService( Service* pService);
    
	bool start(bool startInSeparateThread);
    void stop();
    void pause();
    void resume();

public:
	MessageFactory& getMessageFactory();
    Server* getFacade();
	string getServerInfos();

private:
    Server* pFacade;
    string serverInfos;

    // User defined
	MessageFactory* pMsgFactory;

#ifdef Plateform_Windows
    HANDLE			m_hEventTimer;	// Used for sleeping during interval period.
    HANDLE			m_hEventAbort; // set by stop function.
    //In case of an asynchronous start :
    HANDLE hThread;
    static DWORD WINAPI threadProc(LPVOID WorkContext);
#else
#ifdef Plateform_Linux
    pthread_cond_t abortCond;// set by stop function.
    //In case of an asynchronous start :
    pthread_t hThread;
    static void* threadProc(void* WorkContext);
#endif
#endif

private:
    unsigned int uGCPeriod;
    int nSecsGCCount;
    int nSecsPerfObservCount;

	void eternal_loop();
    void DoPeriodicJobs();

private:
	typedef std::vector<BroadcastStreamer*> streamersListT;
	streamersListT streamersList;

	typedef unordered_map<int, Listener*> listenersMapT;
	listenersMapT listenersMap;

public:
	void startStreamers();
	void stopStreamers();
	void reshuffleStreamers();

	void addClientToStreamers(LogicalConnectionImpl* pLogicalConnection);

	void CreateQueue(BROADCASTQUEUE_NAME queueName, QueueOptions& queueOptions);
	void RemoveQueue(BROADCASTQUEUE_NAME queueName);
	bool PushPacket(OutgoingPacket* pPacket, BROADCASTQUEUE_NAME queueName);
	bool PushPacket(OutgoingPacket* pPacket, BROADCASTQUEUE_NAME queueName, BROADCASTPACKET_KEY killKey, int objectCategory);
	void RemovePacketFromQueue(BROADCASTPACKET_KEY killKey, int objectCategory, BROADCASTQUEUE_NAME queueName);

	std::string getListeners();
private:
	bool StartListening();
	void StopListening();
    void DestroyListeners();

private:
	bool initializeProtocolContextPools(unsigned int nMaxPoolConnections);
	void uninitializeProtocolContextPools();

private:
	IOQueue* pIOQueue;
	DemuxImpl* pDemux;
	ClientFactory* pClientFactory;
	Dispatcher* pDispatcher;
	GarbageCollector* pGarbageCollector;
	ProtocolManager* pProtocolMgr;
	BroadcastManager* pBroadcastMgr;
	ServerStats* pServerStats;
	ServerOptions* pServerOptions;
	MemoryAllocator* pMsgMemAllocator;
	MonitorServer* pMonitorServer;
	
public:
	IOQueue& getIOQueue();
	DemuxImpl& getDemux();
	ClientFactory& getClientFactory();
	Dispatcher& getDispatcher();
	GarbageCollector& getGarbageCollector();
	ProtocolManager& getProtocolMgr();
	BroadcastManager& getBroadcastManager();
	ServerStats& getServerStats();
    MemoryAllocator* getAllocator();


private:
	LogicalConnectionPool* pLogicalConnectionPool;
	ConnectionContextPool* pConnectionContextPool;
	PhysicalConnectionPool* pPhysicalConnectionPool;
	

public:
	LogicalConnectionPool& getLogicalConnectionPool();
	ConnectionContextPool& getConnectionContextPool();
	PhysicalConnectionPool& getPhysicalConnectionPool();
	ServerOptions& getServerOptions();
	MonitorServer& getMonitorServer();


};

}
#endif // ServerImpl__INCLUDED

