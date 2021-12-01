#include "Symbols.h"

// This class is exported from the JsonSerializer.dll
class JSONSERIALIZER_API JsonSerializer  : public MessageFactory
{
public:
	JsonSerializer(void)
	{
		//
	}
	~JsonSerializer()
	{

	}

    virtual SerializeResult::Type serializeMessage(OutgoingPacket& outgoingPacket, Buffer& buffer);

	virtual DeserializeResult::Type deserializeMessage( Buffer& contentBytes, IncomingPacket*& pMessage, int& nRoutingService );

	virtual void disposeIncomingPacket( IncomingPacket* pPacket );

	virtual void disposeOutgoingPacket( OutgoingPacket* pPacket );
};

