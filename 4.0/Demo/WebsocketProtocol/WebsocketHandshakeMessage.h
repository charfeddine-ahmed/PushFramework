#pragma once
class WebsocketHandshakeMessage
{
public:
	WebsocketHandshakeMessage();
	~WebsocketHandshakeMessage(void);


	bool ParseFromRequest(Buffer& request);

	string GetField( string key );

	void SetField( string key, string val );


	bool Serialize(Buffer& response);
private:
	string m_method;
	string m_uri;
	string m_version;
	map<string, string> fields;
};

