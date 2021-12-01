#include "stdafx.h"
#include "Buffer.h"
#include "MemoryAllocator.h"
#include "MemorySegment.h"

namespace ProtocolFramework
{

    Buffer::Buffer(MemoryAllocator* pAllocator)
	{
        this->pAllocator = pAllocator;
		//
        ownsBuffer = false;
        segment = NULL;
        size = 0;
        offset = 0;
	}

    Buffer::Buffer(MemoryAllocator* pAllocator, unsigned int _size)
    {
        this->pAllocator = pAllocator;

        ownsBuffer = false;
        segment = NULL;
        size = 0;
        offset = 0;

        allocate(_size);
    }

	Buffer::~Buffer( void )
	{
        release();
	}

    inline unsigned int Buffer::getDataSize()
    {
        return size;
    }

    inline unsigned int Buffer::getRemainingSize()
    {
        return getCapacity() - getDataSize();
    }

    inline unsigned int Buffer::getCapacity()
    {
        return segment->capacity;
    }

    inline char* Buffer::getPosition()
    {
        return segment->data + offset + size;
    }

    inline void Buffer::clearBytes()
    {
        size = 0;
        offset = 0;
    }

    inline bool Buffer::isEmpty()
    {
        return size == 0;
    }

    inline bool Buffer::hasBytes()
    {
        return size > 0;
    }

    inline void Buffer::setPosition(unsigned int size)
    {
        this->size = size;
    }

    inline bool Buffer::isFull()
    {
        return getRemainingSize() == 0;
    }

    inline char* Buffer::getBuffer(int _offset/* = 0*/)
    {
        return segment->data + offset + _offset;
    }

    inline char Buffer::getAt(int _offset)
    {
        return segment->data[offset + _offset];
    }

    inline void Buffer::growSize(unsigned int growBy)
    {
        size += growBy;
    }

    inline bool Buffer::append(const char* _buf, unsigned int _size)
    {
        assert(segment != NULL);

        if (getRemainingSize() < _size)
            return false;

        ::memcpy(segment->data + offset + size, _buf, _size);

        size += _size;
        return true;
    }

    inline bool Buffer::append(const char c)
    {
        return append(&c, 1);
    }

    inline bool Buffer::append(Buffer& srcBuffer)
    {
        return append(srcBuffer.getBuffer(), srcBuffer.getDataSize());
    }

    inline bool Buffer::append(Buffer& srcBuffer, unsigned int size)
    {
        return append(srcBuffer.getBuffer(), size);
    }

    inline void Buffer::pop(unsigned int _size)
    {
        assert(size >= _size);

        size -= _size;
        
        offset += _size;
        //::memmove(segment->data + offset, segment->data + offset + _size, size);
    }

    void Buffer::assign(Buffer& srcBuffer, bool takeOwnership)
	{
        assert(segment == NULL);

        segment = srcBuffer.segment;
        offset = srcBuffer.offset;
        size = srcBuffer.size;

        if (takeOwnership)
        {
            if (!srcBuffer.ownsBuffer)
            {
                assert(false); // Impossible to own a buffer that does not own its segment.
            }
            
            ownsBuffer = true;
            srcBuffer.segment = NULL;
            srcBuffer.release();
        }
        else
        {
            ownsBuffer = false;
        }
	}

    bool Buffer::allocate(unsigned int _size)
    {
        // assert(segment == NULL);
        if (segment)
        {
            if (segment->capacity > _size)
            {
                clearBytes();
                return true;
            }
            else
                release();
        }

        segment = pAllocator->getMemorySegment(_size);
        ownsBuffer = true;
        return true;
    }

    MemoryAllocator* Buffer::getAllocator()
    {
        return pAllocator;
    }

    void Buffer::release()
    {
        if (segment && ownsBuffer)
        {
            pAllocator->returnMemorySegment(segment);
        }

        segment = NULL;
        ownsBuffer = false;
        size = 0;
        offset = 0;
    }

    bool Buffer::setData(const char* _buf, unsigned int _size)
    {
        if (segment)
        {
            if (_size > segment->capacity)
            {
                assert(false);
                return false;
            }
        }
        else
        {
            allocate(_size);
        }

        return append(_buf, _size);
    }

    

}


