#include "stdafx.h"
#include "EchoService2.h"
#include "DemoClient.h"

EchoService2::EchoService2(void)
{
}


EchoService2::~EchoService2(void)
{
}

void EchoService2::handle( LogicalConnection* pClient, IncomingPacket* pRequest )
{
	Json::Value& jsonVal = reinterpret_cast<Json::Value&> (*pRequest);
	DemoClient& demoClient = dynamic_cast<DemoClient&> (*pClient);

	cout << "received echo 2 req" << endl;

	demoClient.PushJsonPacket(jsonVal);
}

unsigned int EchoService2::getRoutingId()
{
	return Demo::Echo2;
}

const char* EchoService2::getName()
{
	return "echo2";
}
