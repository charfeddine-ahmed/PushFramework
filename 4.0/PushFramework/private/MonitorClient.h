#pragma once
#include "../include/LogicalConnection.h"
namespace PushFramework
{
	class MonitorServer;
	class MonitorClient : public LogicalConnection
	{
	public:
		MonitorClient(MonitorServer* pServer);
		~MonitorClient(void);

		virtual void handleRequest( IncomingPacket* pRequest );

		virtual void Recycle();

		virtual Login::Type processLogin( LoginData& loginData );

		virtual void OnDisconnected( DisconnectionReason::Type closeReason );

		virtual void OnConnected();

	private:
		MonitorServer* pServer;
	};

}
