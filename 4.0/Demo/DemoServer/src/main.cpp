// DemoServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DemoServer.h"
#include "EchoService1.h"
#include "EchoService2.h"

int main(int argc, char* argv[])
{
	if (!readConfig())
	{
		cout << "Unable to read configuration file" << endl;
		return -1;
	}

	theServer.getOptions().isMonitorEnabled = true;
	theServer.getOptions().challengeClients = false;
	theServer.getOptions().isProfilingEnabled = true;
	theServer.getOptions().monitorPort = theDemoServerOptions.monitorPort;
	strcpy(theServer.getOptions().password, theDemoServerOptions.password.c_str());
	theServer.getOptions().nMaxConnections = 50;
	theServer.getOptions().samplingRate = 10;
	theServer.getOptions().nStreamers = 1;
	theServer.getOptions().uMaxClientIdleTime = 60;


	JsonSerializer jsonSerializer;
	SimpleProtocol simpleProtocol;

	theServer.setMessageFactory(&jsonSerializer);

	EchoService1 echoService1;
	EchoService2 echoService2;
	theServer.registerService(&echoService1);
	theServer.registerService(&echoService2);

	ListenerOptions lOptions;
	lOptions.pProtocol = &simpleProtocol;

	theServer.createListener(theDemoServerOptions.port, &lOptions);


	QueueOptions options;
	//
	options.maxPackets = 100;
	options.requireRegistration = true;
	options.priority = 10;
	options.packetsQuota = 10;
	options.fillRateThrottlingPeriod = 60;
	options.fillRateThrottlingMaxPackets = 100;

	theServer.CreateQueue("queue1", options);

	options.maxPackets = 1000;
	options.priority = 5;
	options.packetsQuota = 5;

	theServer.CreateQueue("queue2", options);


	theServer.start(true);

	Sleep(1000);

	cout << "DemoServer started. press q to stop" << endl;
	while (true)
	{
		char ch;

		cin >> ch;

		if (ch == 'q')
		{
			break;
		}
	}

	theServer.stop();

	return 0;
}

