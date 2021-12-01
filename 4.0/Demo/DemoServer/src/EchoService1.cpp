#include "stdafx.h"
#include "EchoService1.h"
#include "DemoClient.h"

EchoService1::EchoService1(void)
{
}


EchoService1::~EchoService1(void)
{
}

void EchoService1::handle( LogicalConnection* pClient, IncomingPacket* pRequest )
{
	Json::Value& jsonVal = reinterpret_cast<Json::Value&> (*pRequest);
	DemoClient& demoClient = dynamic_cast<DemoClient&> (*pClient);

	cout << "received echo 1 req" << endl;

	demoClient.PushJsonPacket(jsonVal);

	/*Json::Value reply;*/
}

unsigned int EchoService1::getRoutingId()
{
	return Demo::Echo1;
}

const char* EchoService1::getName()
{
	return "echo1";
}
