#pragma once
class WebsocketControlMessage
{
public:
	enum Type
	{
		Handshake,
		Ping,
		Pong
	};
	WebsocketControlMessage(Type type)
	{
		this->type = type;
	}
	WebsocketControlMessage(void);
	~WebsocketControlMessage(void);

private:
	Type type;
};

