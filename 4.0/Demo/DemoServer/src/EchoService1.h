#pragma once
class EchoService1 : public Service
{
public:
	EchoService1(void);
	~EchoService1(void);

	virtual void handle( LogicalConnection* pClient, IncomingPacket* pRequest );

	virtual unsigned int getRoutingId();

	virtual const char* getName();

};

