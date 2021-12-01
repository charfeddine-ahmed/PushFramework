// SimpleProtocol.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SimpleProtocol.h"

#define SignatureStart 16018
#define SignatureEnd 16108

SimpleProtocol::SimpleProtocol()
{
	return;
}

SimpleProtocol::~SimpleProtocol( void )
{
	//
}

EncodeResult::Type SimpleProtocol::encodeContent( ProtocolContext& context, Buffer& inputBuffer, Buffer& outputBuffer )
{
    unsigned int nDataSize = inputBuffer.getDataSize();
	unsigned int nPacketSize = 4 + nDataSize+ 2;

    if (!outputBuffer.allocate(nPacketSize))
        return EncodeResult::FatalFailure;

	unsigned int sStart = SignatureStart;	
    outputBuffer.append((char*) &sStart, 2);
	outputBuffer.append((char*) &nDataSize, 2);

	outputBuffer.append(inputBuffer);

	unsigned int sEnd = SignatureEnd;
	outputBuffer.append((char*)&sEnd, 2);


	return EncodeResult::Success;
}

DecodeResult::Type SimpleProtocol::tryDecode( ProtocolContext& context, Buffer& outputBuffer )
{
	BufferList& inputbuffer = context.getDataBuffer();

    unsigned int nMinimumRequiredSize = 4 + 2;
	if (inputbuffer.getDataSize() < nMinimumRequiredSize)
		return DecodeResult::WantMoreData;

	unsigned int sStart = 0;
	unsigned int nDataSize = 0;
	unsigned int sEnd = 0;

	for (int i=0; i<2; i++)
	{
        *((char*)(&sStart) + i) = inputbuffer.getAt(i);
		*((char*)(&nDataSize)+i)= inputbuffer.getAt(i+2);
	}

	if (sStart != SignatureStart)
		return DecodeResult::Failure;

	outputBuffer.clearBytes();

    nMinimumRequiredSize += nDataSize;
    if (inputbuffer.getDataSize() < nMinimumRequiredSize)
		return DecodeResult::WantMoreData;

	for (int i=0; i<2; i++)
		*((char*)(&sEnd)+i) = inputbuffer.getAt(4 + nDataSize + i);

	if( sEnd != SignatureEnd )
		return DecodeResult::Failure;

    if (!outputBuffer.allocate(nDataSize))
        return DecodeResult::Failure;

    inputbuffer.pop(4);

    if (!inputbuffer.popToBuffer(outputBuffer, nDataSize))
        return DecodeResult::Failure;

    inputbuffer.pop(2);

	return DecodeResult::Content;
}

void SimpleProtocol::startSession( ProtocolContext& context, Buffer& outgoingBytes )
{
	context.setInitialized();
}

ProtocolContext* SimpleProtocol::createNewProtocolContext()
{
	return new ProtocolContext();
}

bool SimpleProtocol::readData( ProtocolContext& context, Buffer& incomingBytes )
{
	context.getDataBuffer().addBuffer(incomingBytes);
    return true;
}
