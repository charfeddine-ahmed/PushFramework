/********************************************************************
        File :			Dispatcher.cpp
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
#include "Dispatcher.h"


#include "../include/ListenerOptions.h"
#include "../include/LogicalConnection.h"
#include "../include/Service.h"
#include "../include/Server.h"
#include "../include/ServerOptions.h"

#include "ScopedLock.h"
#include "PhysicalConnection.h"
#include "ServerImpl.h"
#include "ServerStats.h"
#include "XMLPacket.h"
#include "ClientFactory.h"
#include "BroadcastManager.h"
#include "BroadcastQueue.h"
#include "StopWatch.h"
#include "Utilities.h"
#include "Listener.h"
#include "MonitorsMsgFactory.h"
#include "ProtocolManager.h"
#include "LogicalConnectionImpl.h"
#include "LogicalConnectionPool.h"
#include "MonitorServer.h"

namespace PushFramework
{

Dispatcher::Dispatcher(ServerImpl* pServerImpl)
{
	this->pServerImpl = pServerImpl;
    ::InitializeCriticalSection(&csSrvMap);
}

Dispatcher::~Dispatcher(void)
{
    ::DeleteCriticalSection(&csSrvMap);
}

std::string Dispatcher::getServiceNames()
{
    std::stringstream ss;
    ss << std::noskipws;

    for (serviceMapT::iterator it = serviceMap.begin();
            it != serviceMap.end();
            it++)
    {
		ss << "<service>";
        ss << "<name val=\"" << it->second->getName() << "\"/>";
		ss << "<routingId val=\"" << it->second->getRoutingId() << "\"/>";
		ss << "</service>";
    }

    return ss.str();
}

void Dispatcher::setCurrentService(std::string serviceName)
{
    ScopedLock lock(csSrvMap);
    ThreadIDType dwThread = Utilities::getCurrentThreadID();

    workerServiceMap[dwThread] = serviceName;
}

void Dispatcher::unsetCurrentService()
{
    ScopedLock lock(csSrvMap);
    ThreadIDType dwThread = Utilities::getCurrentThreadID();

    workerServiceMapT::iterator it = workerServiceMap.find(dwThread);
    if (it != workerServiceMap.end())
        workerServiceMap.erase(it);
}

bool Dispatcher::getCurrentService(std::string& serviceName)
{
    ScopedLock lock(csSrvMap);
    ThreadIDType dwThread = Utilities::getCurrentThreadID();

    workerServiceMapT::iterator it = workerServiceMap.find(dwThread);
    if (it == workerServiceMap.end())
        return false;

    serviceName = it->second;
    return true;
}

void Dispatcher::notifyObserversClientIN(const char* key, std::string peerIP, unsigned int peerPort)
{
	if (!pServerImpl->getServerOptions().isProfilingEnabled)
		return;

    std::string timestamp = Utilities::getCurrentTime();

    XMLPacket *response = new XMLPacket(AnalyticsProtocol::VisitorInResponse);
    response->setArgumentAsText("time", timestamp.c_str());
    response->setArgumentAsText("name", key);
    response->setArgumentAsText("ip", peerIP.c_str());
    response->setArgumentAsInt("port", peerPort);

	pServerImpl->getMonitorServer().PushPacket(response, "clientsIn", key, 0);
}

void Dispatcher::notifyObserversClientOut(const char* key)
{
	if (!pServerImpl->getServerOptions().isProfilingEnabled)
		return;
    
	XMLPacket *response = new XMLPacket(AnalyticsProtocol::VisitorOutResponse);
    response->setArgumentAsText("name", key);

    pServerImpl->getMonitorServer().PushPacket(response, "clientsOut");

    //Remove client from the other broadcast group :
	pServerImpl->getMonitorServer().RemovePacketFromQueue(key, 0, "clientsIn");
}

void Dispatcher::handleInitialize(PhysicalConnection& connection)
{
	Buffer protocolBytes(pServerImpl->getAllocator());
	Protocol* pProtocol = connection.AdvanceInitialization(protocolBytes);
	if (pProtocol)
	{
		connection.pushBytes(protocolBytes, pProtocol->getLowerProtocol());
	}

	connection.CheckConnectionInitialization();

    connection.postReceive();
}

void Dispatcher::registerService(Service* pService)
{
    serviceMap[pService->getRoutingId()] = pService;
}


void Dispatcher::processFirstRequest( PhysicalConnection& connection, IncomingPacket& packet, int nRoutingService, unsigned int serviceBytes )
{
    OutgoingPacket* pOutPacket = NULL;

	LoginData loginData;
	loginData.connectionContext = connection.GetConnectionContext();
	loginData.pRequest = &packet;

	LogicalConnectionImpl* newClient = (LogicalConnectionImpl*) pServerImpl->getLogicalConnectionPool().borrowObject();

	Login::Type type = newClient->processLogin(loginData);

	if (!Login::IsSucceeded(type))
	{
		bool bWaitForSendToComplete = (loginData.pResponse != NULL);
		if (loginData.pResponse)
		{
			connection.pushPacket(loginData.pResponse);
			pServerImpl->getMessageFactory().disposeOutgoingPacket(loginData.pResponse);
		}

		if (type == Login::RefuseAndClose)
		{
			connection.closeConnection(bWaitForSendToComplete);
		}

		pServerImpl->getLogicalConnectionPool().returnObject(newClient);
		return;
	}

	//At this moment, claim the physical connection:
	if (!pServerImpl->getClientFactory().removePhysicalConnection(&connection))
	{
		pServerImpl->getLogicalConnectionPool().returnObject(newClient);
		return;
	}

	newClient->initializeIntern(&connection);

	pServerImpl->addClientToStreamers(newClient);

	//type == Login::AcceptClient.
	pServerImpl->getClientFactory().addLogicalConnection(newClient);

	newClient->OnConnected();

	if (type == Login::AcceptClientAndRouteRequest)
	{
		DispatchRequest(connection, loginData.pRequest, nRoutingService, serviceBytes);
	}
	else
	{
		if (loginData.pResponse)
		{
			connection.pushPacket(loginData.pResponse);
			pServerImpl->getMessageFactory().disposeOutgoingPacket(loginData.pResponse);
		}
	}
	
	//Statistics :
	pServerImpl->getServerStats().addToCumul(ServerStats::VisitorsHitsIn, 1);
	//notifyObserversClientIN(addedClient->getKey(), connection.getPeerIP(), connection.getPeerPort());
	pServerImpl->getServerStats().addToCumul(ServerStats::VisitorsOnline, 1);
    

    newClient->DecrementUsage();
}

void Dispatcher::handleFailedIO( PhysicalConnection& connection )
{
	connection.decrementIoWorkersReferenceCounter();
}

void Dispatcher::handleRead(PhysicalConnection& connection, int dwIoSize)
{
	int status = connection.getStatus();

	bool isFailed = false;
	bool waitForPendingPackets = false;

	Buffer incomingBytes(pServerImpl->getAllocator());
	isFailed = !connection.readReceivedBytes(incomingBytes, dwIoSize);

	if (!isFailed)
	{
		isFailed = !pServerImpl->getProtocolMgr().processIncomingData(connection, incomingBytes);
	}

	//The Processing Loop.
	DeserializeData deserializeData(pServerImpl->getAllocator(), connection.getProtocol()->getLowestProtocol());
	NetworkDeserializeResult::Type result;
	while (!isFailed)
	{
		deserializeData.clear();
		result = pServerImpl->getProtocolMgr().tryDeserializeIncomingPacket(connection, deserializeData);
		
		if (result == NetworkDeserializeResult::ProtocolBytes)
		{	
			if (deserializeData.protocolBytes.hasBytes())
			{
				connection.pushBytes(deserializeData.protocolBytes, deserializeData.pProtocol->getLowerProtocol());
			}

			continue;
		}
		else if (result == NetworkDeserializeResult::Close)
		{
			if (deserializeData.protocolBytes.hasBytes())
			{
				connection.pushBytes(deserializeData.protocolBytes, deserializeData.pProtocol->getLowerProtocol());
			}

			isFailed = true;
			waitForPendingPackets = true;
		}
		else if (result == NetworkDeserializeResult::Content)
		{
			handleRequest(connection, deserializeData.pMessage, deserializeData.nRoutingService, 0);//TODO
			pServerImpl->getMessageFactory().disposeIncomingPacket(deserializeData.pMessage);
		}
		else if(result == NetworkDeserializeResult::WantMoreData)
		{
			connection.postReceive();
			break;
		}
		else
			/*result == NetworkDeserializeResult::Failure or 
			NetworkDeserializeResult::Initializationfailure*/
		{
			isFailed = true;
		}		
	}

	if (!isFailed)
	{
		connection.CheckConnectionInitialization();
	}
	
	if (isFailed)
	{
		if (status == PhysicalConnection::Attached)
		{
			pServerImpl->getClientFactory().disconnect(connection.getLogicalConnectionImpl(), waitForPendingPackets, DisconnectionReason::PeerClosure);
		}
		else
		{
			connection.closeConnection(waitForPendingPackets);
		}
	}

	connection.decrementIoWorkersReferenceCounter();
}

void Dispatcher::handleWrite( PhysicalConnection& connection, int dwIoSize )
{
	bool bIsBufferIdle = false;
	int status = connection.getStatus();

	if (!connection.OnSendCompleted(dwIoSize, bIsBufferIdle))
	{
		if (status == PhysicalConnection::Attached)
		{
			pServerImpl->getClientFactory().disconnect(connection.getLogicalConnectionImpl(), false, DisconnectionReason::PeerClosure);
		}
		else
		{
			connection.closeConnection(false);
		}

		connection.decrementIoWorkersReferenceCounter();
		return;
	}

	if (status == PhysicalConnection::Attached)
	{
		if (bIsBufferIdle)
		{
			connection.getLogicalConnectionImpl()->CheckAndProcessPendingBroadcast(false);
		}

		connection.getLogicalConnectionImpl()->OnReadyForSend();
	}

	connection.decrementIoWorkersReferenceCounter();
}


void Dispatcher::handleRequest( PhysicalConnection& connection, IncomingPacket* pPacket, int nRoutingService, unsigned int serviceBytes )
{
	if (connection.getStatus() == PhysicalConnection::Connected)
	{
		processFirstRequest(connection, *pPacket, nRoutingService, serviceBytes);
		return;
	}

	DispatchRequest(connection, pPacket, nRoutingService, serviceBytes);
	return;
}

void Dispatcher::DispatchRequest( PhysicalConnection &connection, IncomingPacket* pPacket, int nRoutingService, unsigned int serviceBytes )
{
	if (connection.getStatus() != PhysicalConnection::Attached)
	{
		return;
	}

	LogicalConnectionImpl* pClient = connection.getLogicalConnectionImpl();

	//
	serviceMapT::iterator it = serviceMap.find(nRoutingService);
	if (it == serviceMap.end())
	{
		pClient->getFacade()->handleRequest(pPacket);
		return;
	}
	//

	Service* pService = it->second;

	setCurrentService(pService->getName());

	StopWatch watch;
	pService->handle(pClient->getFacade(), pPacket);


	double duration = watch.getElapsedTime();
	pServerImpl->getServerStats().addToDistribution(ServerStats::PerformanceProcessingTimePerService, pService->getName(), duration);

	pServerImpl->getServerStats().addToDuration(ServerStats::PerformanceProcessingTime, duration);

	unsetCurrentService();

	//Stats. :

	pServerImpl->getServerStats().addToDistribution(ServerStats::BandwidthInboundVolPerRequest, pService->getName(), serviceBytes);
	pServerImpl->getServerStats().addToDistribution(ServerStats::PerformanceRequestVolPerRequest, pService->getName(), 1);
}

}

