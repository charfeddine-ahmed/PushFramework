#pragma once
class EchoService2 : public Service
{
public:
	EchoService2(void);
	~EchoService2(void);

	virtual void handle( LogicalConnection* pClient, IncomingPacket* pRequest );

	virtual unsigned int getRoutingId();

	virtual const char* getName();

};

