#pragma once
class WebsocketContext : public ProtocolContext
{
public:
	WebsocketContext();
	~WebsocketContext(void);

	virtual void recycle();

	BufferList bufferedContent;
	int currentContinuationOpCode;
};

