#pragma once
#include "ProtocolFramework.h"

namespace ProtocolFramework
{
    struct PROTOCOLFRAMEWORK_API MemorySegment
	{
		char* data;
        unsigned int capacity;
	};
}