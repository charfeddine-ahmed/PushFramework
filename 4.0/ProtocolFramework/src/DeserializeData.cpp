#include "stdafx.h"
#include "DeserializeData.h"

namespace ProtocolFramework
{
	//

    DeserializeData::DeserializeData(MemoryAllocator* pAllocator, Protocol* pProtocol)
        :protocolBytes(pAllocator)
	{
		this->pProtocol = pProtocol;
		clear();
	}

	DeserializeData::~DeserializeData( void )
	{
		//
	}

	void DeserializeData::clear()
	{
        protocolBytes.release();
		pMessage = NULL;
		nRoutingService = 0;
	}

}
