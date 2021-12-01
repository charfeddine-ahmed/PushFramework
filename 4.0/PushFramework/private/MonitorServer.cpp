#include "StdAfx.h"
#include "MonitorServer.h"
#include "MonitorClient.h"
#include "../include/QueueOptions.h"
#include "../include/ServerOptions.h"
#include "../include/ListenerOptions.h"

namespace PushFramework
{
	MonitorServer::MonitorServer(ServerStats& _pfStats)
		:pfStats(_pfStats)
	{
	}

	MonitorServer::~MonitorServer(void)
	{
	}

	OutgoingPacket* MonitorServer::getChallenge( ConnectionContext* pConnectionContext )
	{
		return NULL;
	}

	LogicalConnection* MonitorServer::createLogicalConnection()
	{
		return new MonitorClient(this);
	}

	ConnectionContext* MonitorServer::createConnectionContext()
	{
		return NULL;
	}

	bool MonitorServer::configure( unsigned int port, const char* password )
	{
		this->password = password;

		// Set options:
		getOptions().challengeClients = false;
		getOptions().isMonitorEnabled = false;
		getOptions().isProfilingEnabled = false;
		getOptions().nMaxConnections = 50;
		getOptions().nMaxPendingOutgoingMessages = 5;
		getOptions().nStreamers = 1;
		getOptions().nWorkersCount = 1;
		strcpy(getOptions().password, password);
		getOptions().uLoginExpireDuration = 40;
		getOptions().uMaxClientIdleTime = 60*3;
		getOptions().nSocketBufferSize = 8064;

		//
		setMessageFactory(&theMonitorsMsgFactory);

		//
		ListenerOptions lOptions;
		lOptions.pProtocol = &theMonitorsProtocol;

		createListener(port, &lOptions);

		QueueOptions options;
		//
		options.maxPackets = 100;
		options.requireRegistration = true;
		options.priority = 10;
		options.packetsQuota = 10;
		options.fillRateThrottlingPeriod = 60;
		options.fillRateThrottlingMaxPackets = 100;

		CreateQueue("stats", options);

		options.maxPackets = 1000;
		options.priority = 5;
		options.packetsQuota = 5;

		CreateQueue("clientsIn", options);

		options.maxPackets = 50;

		CreateQueue("clientsOut", options);

		return start(true);
	}

	const char* MonitorServer::getPassword()
	{
		return password.c_str();
	}

	ServerStats& MonitorServer::getStats()
	{
		return pfStats;
	}

	MonitorsMsgFactory& MonitorServer::getMonitorsMsgFactory()
	{
		return theMonitorsMsgFactory;
	}

}
