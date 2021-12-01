
#include "Symbols.h"

class SIMPLEPROTOCOL_API SimpleProtocol : public Protocol
{
public:
	SimpleProtocol(void);
	~SimpleProtocol(void);

protected:

	virtual ProtocolContext* createNewProtocolContext();
	virtual void startSession(ProtocolContext& context, Buffer& outgoingBytes);
	virtual bool readData(ProtocolContext& context, Buffer& incomingBytes);
	virtual DecodeResult::Type tryDecode(ProtocolContext& context, Buffer& outputBuffer);
	virtual EncodeResult::Type encodeContent(ProtocolContext& context, Buffer& inputBuffer, Buffer& outputBuffer);

};

