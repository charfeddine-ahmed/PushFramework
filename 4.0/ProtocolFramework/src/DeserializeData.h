#pragma once
#include "ProtocolFramework.h"
#include "Buffer.h"

namespace ProtocolFramework
{
	class Protocol;
	class IncomingPacket;

	struct PROTOCOLFRAMEWORK_API DeserializeData
	{
        DeserializeData(MemoryAllocator* pAllocator, Protocol* pProtocol);
		~DeserializeData(void);

		void clear();
		Protocol* pProtocol;
		IncomingPacket* pMessage;
		int nRoutingService;
        Buffer protocolBytes;
	};
}
