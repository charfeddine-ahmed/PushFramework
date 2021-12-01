#include "stdafx.h"
#include "MemoryAllocator.h"
#include "MemorySegment.h"

namespace ProtocolFramework
{
    MemoryAllocator::MemoryAllocator()
    {
    }


    MemoryAllocator::~MemoryAllocator()
    {
    }

    MemorySegment* MemoryAllocator::getMemorySegment(unsigned int nSize)
    {
        MemorySegment* segment = new MemorySegment();
        segment->data = new char[nSize];
        segment->capacity = nSize;
        return segment;
    }

    void MemoryAllocator::returnMemorySegment(MemorySegment* segment)
    {
        delete[] segment->data;
        delete segment;
    }

}