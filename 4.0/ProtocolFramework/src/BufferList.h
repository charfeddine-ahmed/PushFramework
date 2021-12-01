#pragma once
#include "ProtocolFramework.h"
#include "Buffer.h"

namespace ProtocolFramework
{
    class PROTOCOLFRAMEWORK_API BufferList
    {
    public:
        BufferList();
        ~BufferList();

        bool isEmpty();

        unsigned int getDataSize();

        char getAt(unsigned int offset);

        bool popToBuffer(Buffer& buffer, unsigned int size);

        void pop(unsigned int size);

        void release();

        void addBuffer(Buffer& buffer);

        void copyTo(unsigned int offset, char* dst, unsigned int size);

        bool copyTo(Buffer& buffer, unsigned int size);

        void setAt(unsigned int offset, char val);

        unsigned int getBufferCount();

    private:
        struct Internal;
        Internal* internal;
    };
}
