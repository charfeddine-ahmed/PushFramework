#include "stdafx.h"
#include "DemoServer.h"
#include "DemoClient.h"

DemoServer::DemoServer(void)
{
}


DemoServer::~DemoServer(void)
{
}

OutgoingPacket* DemoServer::getChallenge( ConnectionContext* pConnectionContext )
{
	return NULL;
}

LogicalConnection* DemoServer::createLogicalConnection()
{
	return new DemoClient();
}


DemoServer theServer;