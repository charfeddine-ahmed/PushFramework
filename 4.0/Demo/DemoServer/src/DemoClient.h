#pragma once
class DemoClient : public LogicalConnection
{
public:
	DemoClient(void);
	~DemoClient(void);

	virtual void Recycle();

	virtual Login::Type processLogin( LoginData& loginData );

public:
	SendResult::Type PushJsonPacket(Json::Value& val);
};

