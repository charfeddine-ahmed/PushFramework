#pragma once
class DemoServer : public Server
{
public:
	DemoServer(void);
	~DemoServer(void);

	virtual OutgoingPacket* getChallenge( ConnectionContext* pConnectionContext );

	virtual LogicalConnection* createLogicalConnection();
};

extern DemoServer theServer;