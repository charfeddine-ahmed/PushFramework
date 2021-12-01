#pragma once
#include "ProtocolFramework.h"


namespace ProtocolFramework
{
    struct MemorySegment;

    class PROTOCOLFRAMEWORK_API MemoryAllocator
    {
    public:
        MemoryAllocator();
        ~MemoryAllocator();
        
        MemorySegment* getMemorySegment(unsigned int nSize);
        
        void returnMemorySegment(MemorySegment* segment);
    };

}