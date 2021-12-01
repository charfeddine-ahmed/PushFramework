#include "Symbols.h"

// This class is exported from the WebsocketProtocol.dll
class WEBSOCKETPROTOCOL_API WebsocketProtocol : public Protocol
{
public:
	WebsocketProtocol(bool isBinaryMode = true, bool isClientSide = false);

	void ValidateTextAsUtf8(bool val);

protected:
	virtual ProtocolContext* createNewProtocolContext();
	virtual void startSession(ProtocolContext& context, Buffer& outgoingBytes);
    virtual bool readData(ProtocolContext& context, Buffer& incomingBytes);
	virtual DecodeResult::Type tryDecode(ProtocolContext& context, Buffer& outputBuffer);
	virtual DecodeResult::Type tryDecodeHanshakeData(ProtocolContext& context, Buffer& outputBuffer);
	virtual EncodeResult::Type encodeContent(ProtocolContext& context, Buffer& inputBuffer, Buffer& outputBuffer);
	unsigned int getRequiredRecyclableBuffers( unsigned int nMaxConnections, unsigned int nMaxConcurrentCalls );
private:
	bool isBinaryMode;
	bool isClientSide;
	bool validateTextAsUtf8;
private:
	bool processHandshake(Buffer& request, Buffer& response);
	virtual EncodeResult::Type encodeContent(ProtocolContext& context, Buffer& inputBuffer, Buffer& outputBuffer, int opCode);
	bool validateUtf8Context(Buffer& content);
	void validateCloseFramePayload(Buffer& payload);
};
