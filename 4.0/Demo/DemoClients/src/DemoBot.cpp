#include "stdafx.h"
#include "DemoBot.h"
#include "Configuration.h"

DemoBot::DemoBot(void)
{
	isLogged = false;
	nSecs = 0;
	initialize(&simpleProtocol, &jsonSerializer);

	string strErr;
	connect(options.serverIP.c_str(), options.port, strErr);
}


DemoBot::~DemoBot(void)
{
}

void DemoBot::onConnected()
{
	cout << "connected " << endl;

	//Login
	Json::Value loginReq;
	loginReq["routingId"]  = Demo::Login;

	PushJsonPacket(loginReq);
}

void DemoBot::onConnectionClosed()
{
	cout << "disconnected " << endl;
	isLogged = false;
}

void DemoBot::onPerformAutomatedJob()
{
	if (!isLogged)
	{
		return;
	}

	if (options.disableActivity)
	{
		return;
	}

	nSecs++;

	//
	if (nSecs % options.echo1Freq == 0)
	{
		Json::Value req;
		req["routingId"] = Demo::Echo1;
		PushJsonPacket(req);
	}

	if (nSecs % options.echo2Freq == 0)
	{
		Json::Value req;
		req["routingId"] = Demo::Echo2;
		PushJsonPacket(req);
	}
}

void DemoBot::onMessage( ProtocolFramework::IncomingPacket* pMsg )
{
	Json::Value& jsonVal = reinterpret_cast<Json::Value&> (*pMsg);

	int routingId = jsonVal.get("routingId", 0).asInt();

	if (routingId == Demo::Login)
	{
		cout << "successfully logged in" << endl;
		isLogged = true;
		return;
	}

	//
	cout << "received reply " << routingId << endl;
}

int DemoBot::PushJsonPacket(Json::Value& val)
{
	OutgoingPacket* outgoingPacket = reinterpret_cast<OutgoingPacket*> (&val);
	return sendRequest(outgoingPacket);
}