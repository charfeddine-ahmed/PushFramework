#include "stdafx.h"
#include "MessageFactory.h"

namespace ProtocolFramework
{
	MessageFactory::MessageFactory(void)
	{
	}

	MessageFactory::~MessageFactory(void)
	{
	}

	bool ProtocolFramework::MessageFactory::preSerializeMessage( OutgoingPacket& outgoingPacket )
	{
		return true;
	}
}
