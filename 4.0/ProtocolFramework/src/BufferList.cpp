#include "stdafx.h"
#include "BufferList.h"

namespace ProtocolFramework
{
    typedef vector<Buffer*> bufferVectT;

    struct BufferList::Internal
    {
        Internal()
        {
            //
        }
        ~Internal()
        {
            //
        }
        
        bufferVectT bufferVect;
    };

    BufferList::BufferList()
    {
        internal = new Internal();
    }
    
    BufferList::~BufferList()
    {
        delete internal;
    }

    bool BufferList::isEmpty()
    {
        if (internal->bufferVect.empty())
            return true;

        Buffer* first = internal->bufferVect.front();
        return first->isEmpty();
    }

    unsigned int BufferList::getDataSize()
    {
        unsigned int size = 0;
        for (bufferVectT::iterator it = internal->bufferVect.begin();
            it != internal->bufferVect.end();
            it++)
        {
            Buffer* buffer = *it;
            size += buffer->getDataSize();
        }

        return size;
    }

    char BufferList::getAt(unsigned int offset)
    {
        unsigned int size = 0;
        
        for (bufferVectT::iterator it = internal->bufferVect.begin();
            it != internal->bufferVect.end();
            it++)
        {
            Buffer* buffer = *it;

            if (offset < buffer->getDataSize())
                return buffer->getAt(offset);
            
            offset -= buffer->getDataSize();
        }

        //TODO throw exception.
        return '\0';
    }


    bool BufferList::popToBuffer(Buffer& buffer, unsigned int size)
    {
        if (!buffer.allocate(size))
            return false;

        while (true)
        {
            if (internal->bufferVect.empty())
                return true;

            Buffer* _buffer = internal->bufferVect.front();

            unsigned int _size = min(size, _buffer->getDataSize());

            if (!buffer.append(*_buffer, _size))
                return false;

            _buffer->pop(_size);

            if (_buffer->isEmpty())
            {
                delete _buffer;
                internal->bufferVect.erase(internal->bufferVect.begin());
            }

            size -= _size;

            if (size == 0)
                return true;
        }
    }


    void BufferList::addBuffer(Buffer& buffer)
    {
        Buffer* newBuffer = new Buffer(buffer.getAllocator());

        newBuffer->assign(buffer, true);
        
        internal->bufferVect.push_back(newBuffer);
    }

    void BufferList::pop(unsigned int size)
    {
        while (true)
        {
            if (internal->bufferVect.empty())
                return;

            //TODO. declare it.
            Buffer* _buffer = internal->bufferVect.front();

            unsigned int _size = min(size, _buffer->getDataSize());

            _buffer->pop(_size);

            if (_buffer->isEmpty())
            {
                delete _buffer;
                internal->bufferVect.erase(internal->bufferVect.begin());
            }

            size -= _size;

            if (size == 0)
            {
                return;
            }
        }
    }

    void BufferList::copyTo(unsigned int offset, char* dst, unsigned int size)
    {
        unsigned int destOffset = 0;

        for (bufferVectT::iterator it = internal->bufferVect.begin();
            it != internal->bufferVect.end();
            it++)
        {
            Buffer* buffer = *it;

            if (offset > buffer->getDataSize())
            {
                offset -= buffer->getDataSize();
                continue;
            }

            unsigned int nBytes = min(size, buffer->getDataSize());
            memcpy(dst + destOffset, buffer->getBuffer(offset), nBytes);
            destOffset += nBytes;
            size -= nBytes;
            offset = 0;
        }
    }

    bool BufferList::copyTo(Buffer& buffer, unsigned int size)
    {
         for (bufferVectT::iterator it = internal->bufferVect.begin();
            it != internal->bufferVect.end();
            it++)
        {
            Buffer* _buffer = *it;

            unsigned int _size = min(size, _buffer->getDataSize());

            if (!buffer.append(*_buffer, _size))
            {
                return false;
            }

            size -= _size;

            if (size == 0)
                return true;
        }

         return true;
    }

    void BufferList::setAt(unsigned int offset, char val)
    {
        for (bufferVectT::iterator it = internal->bufferVect.begin();
            it != internal->bufferVect.end();
            it++)
        {
            Buffer* _buffer = *it;

            if (offset < _buffer->getDataSize())
            {
                _buffer->getBuffer(offset)[0] = '\0';
                return;
            }
            else
            {
                offset -= _buffer->getDataSize();
            }
        }
    }

    unsigned int BufferList::getBufferCount()
    {
        return internal->bufferVect.size();
    }

    void BufferList::release()
    {
        for (bufferVectT::iterator it = internal->bufferVect.begin();
            it != internal->bufferVect.end();
            it++)
        {
            Buffer* buffer = *it;
            buffer->release();
            delete buffer;
        }

        internal->bufferVect.clear();
    }

}
