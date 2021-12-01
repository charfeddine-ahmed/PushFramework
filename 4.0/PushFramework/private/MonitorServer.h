#pragma once
#include "../include/Server.h"
#include "XMLProtocol.h"
#include "MonitorsMsgFactory.h"

namespace PushFramework
{
	class ServerStats;
	class MonitorServer : public Server
	{
	public:
		MonitorServer(ServerStats& pfStats);
		~MonitorServer(void);

		bool configure(unsigned int port, const char* password);

		virtual OutgoingPacket* getChallenge( ConnectionContext* pConnectionContext );

		virtual LogicalConnection* createLogicalConnection();

		virtual ConnectionContext* createConnectionContext();

		const char* getPassword();

		ServerStats& getStats();

		MonitorsMsgFactory& getMonitorsMsgFactory();

	private:
		XMLProtocol theMonitorsProtocol;
		MonitorsMsgFactory theMonitorsMsgFactory;
		string password;
		ServerStats& pfStats;

	};

}
