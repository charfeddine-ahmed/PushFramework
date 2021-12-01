#include "StdAfx.h"
#include "MonitorClient.h"
#include "XMLPacket.h"
#include "MonitorServer.h"
#include "ServerStats.h"

namespace PushFramework
{
	MonitorClient::MonitorClient(MonitorServer* pServer)
	{
		this->pServer = pServer;
	}


	MonitorClient::~MonitorClient(void)
	{
	}

	void MonitorClient::handleRequest( IncomingPacket* pRequest )
	{
		XMLPacket& requestPacket = (XMLPacket&) (*pRequest);

		int typeId = requestPacket.getTypeId();


		if (typeId == AnalyticsProtocol::ServerInfoRequest)
		{
			OutgoingPacket* pServerInfoRely = pServer->getStats().getInitializationPacket();
			PushPacket(pServerInfoRely);
			pServer->getMonitorsMsgFactory().disposeOutgoingPacket(pServerInfoRely);
			return;
		}

		if (typeId == AnalyticsProtocol::LogoutRequest)
		{
			LogicalConnection::Disconnect(false);
		}

		if (typeId == AnalyticsProtocol::LiveSubscriptionRequest)
		{
			bool bSubscribe = requestPacket.getArgumentAsBool("resume");
			if (bSubscribe)
			{
				SubscribeToQueue("stats");
				SubscribeToQueue("clientsIn");
				SubscribeToQueue("clientsOut");
			}
			else
			{
				UnSubscribeFromAll();
			}
		}

        if (typeId == AnalyticsProtocol::PingRequest)
        {
            //Nothing to do.
        }


		/*

		
		if (typeId == AnalyticsProtocol::ConsoleCommandRequest)
		{
			std::string command = requestPacket.getArgumentAsText("command");

			XMLPacket response(AnalyticsProtocol::ConsoleCommandResponse);

			response.setArgumentAsText("client", requestPacket.getArgumentAsText("client"));

			if (command == "about")
			{
				std::string str = pServerImpl->getServerInfos() + "\nBased on PushFramework version 1.0";
				response.setArgumentAsText("console", str.c_str());
			}

			if (command == "profiling enable")
			{
				std::string str;
				if (pServerImpl->getServerOptions().isProfilingEnabled)
				{
					str = "Profiling is already enabled.";
				}
				else
				{
					pServerImpl->getServerOptions().isProfilingEnabled = false;
					str = "Profiling was enabled.";
				}
				response.setArgumentAsText("console", str.c_str());
			}

			if (command == "profiling disable")
			{
				std::string str;
				if (!pServerImpl->getServerOptions().isProfilingEnabled)
				{
					str = "Profiling is already disabled.";
				}
				else
				{
					pServerImpl->getServerOptions().isProfilingEnabled = false;
					str = "Profiling was disabled.";
				}
				response.setArgumentAsText("console", str.c_str());
			}

			if (command == "profiling status")
			{
				response.setArgumentAsText("console", (!pServerImpl->getServerOptions().isProfilingEnabled) ? "Profiling was found to be disabled." :
					"Profiling was found to be enabled.");
			}

			char pOut[256];

			bool bRet = pServerImpl->getFacade()->handleMonitorRequest(command.c_str(), pOut);

			if (bRet)
			{
				response.setArgumentAsText("console", pOut);
			}

			connection.pushPacket(&response);
		}*/

	}

	void MonitorClient::Recycle()
	{
		//
	}

	Login::Type MonitorClient::processLogin( LoginData& loginData )
	{
		XMLPacket& requestPacket = (XMLPacket&) (*loginData.pRequest);
		std::string password = requestPacket.getArgumentAsText("password");

		bool isPasswordCorrect = strcmp(password.c_str(), pServer->getPassword()) == 0;

		XMLPacket* response = new XMLPacket(AnalyticsProtocol::LoginResponse);
		response->setArgumentAsBool("result", isPasswordCorrect);

		loginData.pResponse = response;
		return isPasswordCorrect ? Login::AcceptClient : Login::RefuseAndWait;
	}

	void MonitorClient::OnDisconnected( DisconnectionReason::Type closeReason )
	{
		//
	}

	void MonitorClient::OnConnected()
	{
		//
	}

}
