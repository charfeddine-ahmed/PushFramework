/********************************************************************
	File :			BroadcastManagerImplBase.cpp
	Creation date :	2012/01/29

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
#include "BroadcastManager.h"

#include "BroadcastQueue.h"
#include "../include/Server.h"
#include "../include/LogicalConnection.h"

#include "ServerStats.h"
#include "ServerImpl.h"
#include "ClientFactory.h"
#include "ServerImpl.h"
#include "BroadcastStreamer.h"
#include "Listener.h"

namespace PushFramework
{


BroadcastManager::BroadcastManager(ServerImpl* pServerImpl)
{
	this->pServerImpl = pServerImpl;
}

BroadcastManager::~BroadcastManager(void)
{
	//Delete Queues : 
	for (channelMapT::iterator it = channelMap.begin(); it!=channelMap.end();it++)
	{
		BroadcastQueue* pQueue = it->second;
		delete pQueue;
	}
}

void BroadcastManager::createBroadcastQueue( BROADCASTQUEUE_NAME channelKey, QueueOptions& queueOptions)
{
    ScopedReadWriteLock(channelMapLock, true);

    BroadcastQueue* pQueue = new BroadcastQueue(this, channelKey, queueOptions);
    channelMap[channelKey] = pQueue;
	if (!queueOptions.requireRegistration)
	{
		globalQueues.push_back(channelKey);
	}
}

void BroadcastManager::removeBroadcastQueue( BROADCASTQUEUE_NAME queueName )
{
    ScopedReadWriteLock(channelMapLock, true);

    channelMapT::iterator it = channelMap.find(queueName);
    if (it == channelMap.end())
        return;

    /*BroadcastQueue* pQueue = it->second;*/

    pServerImpl->getClientFactory().unsubscribeAllFromQueue(queueName);
    
    delete it->second;

    channelMap.erase(it);
}


bool BroadcastManager::pushPacket( OutgoingPacket* pPacket, BROADCASTQUEUE_NAME channelName, BROADCASTPACKET_KEY killKey, int objectCategory )
{
    BroadcastQueue* pQueue = getQueue(channelName);

    if (!pQueue)
    {
        deleteOutgoingPacket(pPacket);
        return false;
    }

    preEncodeOutgoingPacket(pPacket);
    hanldeOnBeforePushPacket(channelName);
    //

    bool bRet = pQueue->pushPacket(pPacket, killKey, objectCategory);
    if (bRet)
    {
        activateSubscribers(channelName);
    }
	else
		deleteOutgoingPacket(pPacket);

    return bRet;
}   

void BroadcastManager::removePacket(BROADCASTPACKET_KEY killKey, int objectCategory, BROADCASTQUEUE_NAME channelName)
{
    BroadcastQueue* pQueue = getQueue(channelName);

    if (!pQueue)
    {
        return;
    }

    pQueue->removePacket(killKey, objectCategory);
}

void BroadcastManager::disposeAllPackets()
{
    for (channelMapT::iterator it = channelMap.begin();
            it!=channelMap.end();
            it++)
    {
        BroadcastQueue* pQueue = it->second;
        pQueue->disposeAllPackets();
    }
}

std::string BroadcastManager::getQueuesNames()
{
    ScopedReadWriteLock(channelMapLock, false);

    std::stringstream ss;
    ss << std::noskipws;

    for (channelMapT::iterator it = channelMap.begin();
            it!=channelMap.end();
            it++)
    {
		BroadcastQueue* pQueue = it->second;

		ss << "<queue>";
        ss << "<name val=\"" << it->first << "\"/>";
		ss << "<priority val=\"" << pQueue->getOptions().priority << "\"/>";
		ss << "<quota val=\"" << pQueue->getOptions().packetsQuota << "\"/>";
		ss << "<requireRegistration val=\"" << pQueue->getOptions().requireRegistration << "\"/>";
		ss << "<size val=\"" << pQueue->getOptions().maxPackets << "\"/>";
		ss << "<fillRateThrottlingPeriod val=\"" << pQueue->getOptions().fillRateThrottlingPeriod << "\"/>";
		ss << "<fillRateThrottlingMaxPackets val=\"" << pQueue->getOptions().fillRateThrottlingMaxPackets << "\"/>";
		ss << "<ignorePreviousPackets val=\"" << pQueue->getOptions().ignorePreviousPackets << "\"/>";
		ss << "<maxExpireDurationSecs val=\"" << pQueue->getOptions().maxExpireDurationSecs << "\"/>";
		ss << "</queue>";
    }

    return ss.str();
}

BroadcastQueue* BroadcastManager::getQueue( BROADCASTQUEUE_NAME channelKey )
{
    ScopedReadWriteLock(channelMapLock, false);

	channelMapT::iterator it = channelMap.find(channelKey);
	if (it==channelMap.end())
		return NULL;

	return it->second;
}

vector<string>& BroadcastManager::getGlobalQueues()
{
	return globalQueues;
}

void BroadcastManager::preEncodeOutgoingPacket( OutgoingPacket* pPacket )
{
	pServerImpl->getMessageFactory().preSerializeMessage(*pPacket);
}

void BroadcastManager::deleteOutgoingPacket( OutgoingPacket* pPacket )
{
	pServerImpl->getMessageFactory().disposeOutgoingPacket(pPacket);
}

void BroadcastManager::activateSubscribers( std::string channelName )
{
	pServerImpl->reshuffleStreamers();
}

void BroadcastManager::hanldeOnBeforePushPacket( std::string channelName )
{
	pServerImpl->getServerStats().addToDistribution(ServerStats::QoSFillRatePerChannel,channelName, 1);
}

void BroadcastManager::handleOnAfterPacketIsSent( std::string channelName, std::string subscriberKey )
{
	pServerImpl->getServerStats().addToDistribution(ServerStats::QoSSendRatePerChannel, channelName, 1);
	pServerImpl->getServerStats().addToKeyedDistributionDuration(ServerStats::QoSAvgSendRatePerChannel, channelName, subscriberKey, 1);
}

}
