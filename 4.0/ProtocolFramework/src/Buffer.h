#ifndef Buffer__INCLUDED
#define Buffer__INCLUDED

#pragma once

#include "ProtocolFramework.h"
#include "MemoryAllocator.h"

namespace ProtocolFramework
{
	// This class holds a line of memory and keeps track of its data. 
	class PROTOCOLFRAMEWORK_API Buffer
	{
	public:
        Buffer(MemoryAllocator* pAllocator);

        Buffer(MemoryAllocator* pAllocator, unsigned int size);

		virtual ~Buffer(void);

		bool allocate( unsigned int _size );

		void assign(Buffer& srcBuffer, bool takeOwnership);
        
        /*void assign(Buffer& srcBuffer, bool takeOwnership, unsigned int _offset, unsigned int _size)
        {
            //
        }*/
       
        bool setData(const char* _buf, unsigned int _size);

		unsigned int getRemainingSize();

		bool append(const char* _buf, unsigned int _size);

		bool append(const char c);

        bool append(Buffer& srcBuffer);
        
        bool append(Buffer& srcBuffer, unsigned int size);

		unsigned int getDataSize();

		unsigned int getCapacity();

		char* getBuffer(int offset = 0);

		char* getPosition();

		char getAt(int offset);

		void growSize(unsigned int growBy);

		void pop(unsigned int _size);

		void clearBytes();

		bool isEmpty();

		bool hasBytes();

		void setPosition(unsigned int size);

		bool isFull();

        MemoryAllocator* getAllocator();

        void release();

	protected:
        MemoryAllocator* pAllocator;
        MemorySegment* segment;
        bool ownsBuffer;
        unsigned int size;
        unsigned int offset;
	};

}
#endif // Buffer__INCLUDED
