#include "stdafx.h"
#include "DemoClient.h"


DemoClient::DemoClient(void)
{
}


DemoClient::~DemoClient(void)
{
}

void DemoClient::Recycle()
{
	//
}

Login::Type DemoClient::processLogin( LoginData& loginData )
{
	cout << "received login" << endl;

	Json::Value *pReply = new Json::Value();
	Json::Value& reply = *pReply;
	reply["routingId"] = Demo::Login;
	
	loginData.pResponse = reinterpret_cast<ProtocolFramework::OutgoingPacket*> (pReply);

	return Login::AcceptClient;
}

SendResult::Type DemoClient::PushJsonPacket(Json::Value& val)
{
	OutgoingPacket* outgoingPacket = reinterpret_cast<OutgoingPacket*> (&val);
	return PushPacket(outgoingPacket);
}