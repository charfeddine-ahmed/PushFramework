#pragma once
class DemoBot : public TCPSocket
{
public:
	DemoBot(void);
	~DemoBot(void);

protected:
	virtual void onConnected();

	virtual void onConnectionClosed();

	virtual void onPerformAutomatedJob();

	virtual void onMessage( ProtocolFramework::IncomingPacket* pMsg );

public:
	int PushJsonPacket(Json::Value& val);

private:
	SimpleProtocol simpleProtocol;
	JsonSerializer jsonSerializer;
	int nSecs;
	bool isLogged;
};

