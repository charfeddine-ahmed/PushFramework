// WebsocketProtocol.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "WebsocketProtocol.h"
#include "WebsocketHandshakeMessage.h"
#include "base64/base64.h"
#include "sha1/sha1.h"
#include "WebsocketContext.h"
#include "utf8.h"

namespace OpCode
{
	enum Type
	{
		Continuation = 0,
		Text = 1,
		Binary = 2,
		Close = 8,
		Ping = 9,
		Pong = 10,
	};

	BYTE getPayloadFlag(bool isFIN, Type type)
	{
		BYTE opCode = type;
		BYTE payloadFlags = isFIN ? (0x80 | opCode) : opCode;
		return payloadFlags;
	}

	void readPayloadFlags(BYTE payloadFlags, bool& isFIN, Type& type)
	{
		isFIN = payloadFlags & 0x80;
		BYTE opCode = payloadFlags & 0x0F;
		type = (Type) opCode;
	}
}

WebsocketProtocol::WebsocketProtocol( bool isBinaryMode /*= true*/, bool isClientSide /*= false*/ )
{
	this->isBinaryMode = isBinaryMode;
	this->isClientSide = isClientSide;
	validateTextAsUtf8 = true;
}

ProtocolContext* WebsocketProtocol::createNewProtocolContext()
{
	return new WebsocketContext();
}

void WebsocketProtocol::startSession( ProtocolContext& context, Buffer& outgoingBytes )
{
	if (isClientSide)
	{
		WebsocketHandshakeMessage handshakeReq;

		handshakeReq.SetField("Sec-WebSocket-Version", "13");
		handshakeReq.SetField("Sec-WebSocket-Key", "uikmSAU+xlcxDKO4grfBRA==");
		handshakeReq.SetField("Upgrade", "websocket");

		handshakeReq.Serialize(outgoingBytes);
	}
	else
	{
		// We do not call websocketContext.setInitialized()
		//Rather we should wait for handshake to complete.
	}
}

bool WebsocketProtocol::readData(ProtocolContext& context, Buffer& incomingBytes)
{
    //TODO.
    context.getDataBuffer().addBuffer(incomingBytes);
    return true;
}

DecodeResult::Type WebsocketProtocol::tryDecode( ProtocolContext& context, Buffer& outputBuffer )
{
	if (!context.isInitialized())
		return tryDecodeHanshakeData(context, outputBuffer);

	//Data is not handshake:

	WebsocketContext& websocketContext = (WebsocketContext&) context;
	BufferList& inputBuffer = context.getDataBuffer();

	unsigned int nExpectedSize = 2;// Flags byte + Size byte

	if (inputBuffer.getDataSize() < nExpectedSize)
		return DecodeResult::WantMoreData;

	OpCode::Type opCode;
	bool isFIN;
	OpCode::readPayloadFlags(inputBuffer.getAt(0), isFIN, opCode);

	if (inputBuffer.getAt(0) & 0x70)
	{
		//RSV bits should all be set to 0.
		return DecodeResult::Close;
	}

	if (opCode != OpCode::Continuation &&
		opCode != OpCode::Text &&
		opCode != OpCode::Binary &&
		opCode != OpCode::Ping &&
		opCode != OpCode::Pong &&
		opCode != OpCode::Close)
		return DecodeResult::Failure;

	bool isControlFrame = opCode == OpCode::Close || opCode == OpCode::Ping || opCode == OpCode::Pong;

	if (!isControlFrame && !isClientSide)
	{
		isBinaryMode = opCode == OpCode::Binary; //TODO.
	}

	if (isControlFrame)
	{
		//Control frames should not be fragmented.
		if(!isFIN || opCode == OpCode::Continuation)
			return DecodeResult::Close;
	}

	if (opCode == OpCode::Continuation)
	{
		//Fail the connection when we are receiving a continuation of nothing.
		if(websocketContext.currentContinuationOpCode == -1)
			return DecodeResult::Failure;
	}

	if (opCode == OpCode::Text || opCode == OpCode::Binary )
	{
		//In middle of fragmentation, only expect continuation frames.
		if(websocketContext.currentContinuationOpCode != -1)
			return DecodeResult::Failure;
	}

	BYTE basicSize = inputBuffer.getAt(1) & 0x7F;
	bool isMasked = inputBuffer.getAt(1) & 0x80;

	//
	if (isMasked)
	{
		nExpectedSize += 4;
		if (inputBuffer.getDataSize() < nExpectedSize)
			return DecodeResult::WantMoreData;
	}

	unsigned __int64 payloadSize = 0;
	int payloadOffset;

	if (basicSize <= 125)
	{
		payloadSize = basicSize;
		payloadOffset = 2;
	}
	else if (basicSize == 126)
	{
		//Add 2 bytes for size:
		nExpectedSize += 2;
		if (inputBuffer.getDataSize() < nExpectedSize)
			return DecodeResult::WantMoreData;

        u_short temp;
        inputBuffer.copyTo(2, (char*) &temp, 2);
		u_short len = ntohs( temp );//*(u_short*)inputBuffer.getBuffer(2)
		payloadSize = len;

		payloadOffset = 4;
	}
	else if (basicSize == 127)
	{
		// Add 8 bytes for size
		nExpectedSize += 8;
		if (inputBuffer.getDataSize() < nExpectedSize)
			return DecodeResult::WantMoreData;

        u_long temp;
        inputBuffer.copyTo(4 + 2, (char*)&temp, 8);
		u_long len = ntohl(temp); //*(u_long*) inputBuffer.getBuffer(2+4)
		payloadSize = len;

		payloadOffset = 10;
	}
	else
		return DecodeResult::Failure;

	// Read masks if any.
	BYTE masks[4];
	if (isMasked)
	{
        inputBuffer.copyTo(payloadOffset, (char*)masks, 4);
		payloadOffset += 4;
	}

	nExpectedSize += payloadSize;
	if (inputBuffer.getDataSize() < nExpectedSize)
		return DecodeResult::WantMoreData;

	if (isControlFrame && payloadSize > 125)
	{
		//Control frames should not have payload size > 125: fail the connection:
		inputBuffer.pop(nExpectedSize);
		return DecodeResult::Close;
	}


	// Handle Pong. (Just do nothing).
	if (opCode == OpCode::Pong)
	{
		inputBuffer.pop(nExpectedSize);
		return DecodeResult::NoContent;
	}


	// Handle both ping and close (share the fact we have to echo back the same payload)
	// If ping is received then send back a pong. In case of a close, send back a close and end the connection.
	if (opCode == OpCode::Ping || opCode == OpCode::Close)
	{
		Buffer controlFrameReply(&getMsgBufferPool());
        inputBuffer.pop(payloadOffset);
        inputBuffer.popToBuffer(controlFrameReply, payloadSize);
		//controlFrameReply.append(inputBuffer.getBuffer(payloadOffset), payloadSize);

		if (isMasked)
		{
			for (unsigned __int64 i = 0; i < payloadSize; i++) {
				controlFrameReply.getBuffer()[i] = (controlFrameReply.getBuffer()[i] ^ masks[i%4]);
			}
		}

		if (opCode == OpCode::Close)
		{
			validateCloseFramePayload(controlFrameReply); //This will empty the reply buffer if if fail validate its content.
		}

		OpCode::Type replyOpCode = (opCode == OpCode::Ping) ? OpCode::Pong : OpCode::Close;

		if (encodeContent(context, controlFrameReply, outputBuffer, replyOpCode) != EncodeResult::Success)
			return DecodeResult::Failure; //Normally, this is impossible case.

		//inputBuffer.pop(nExpectedSize);

		return opCode == OpCode::Close ? DecodeResult::Close : DecodeResult::ProtocolBytes;
	}

	// Handle Data Frames:
    inputBuffer.pop(payloadOffset);

	bool shouldDoBuffering = !isFIN || opCode == OpCode::Continuation;
    Buffer tempBuffer(&getMsgBufferPool());
    Buffer& targetBuffer = shouldDoBuffering ? /*websocketContext.bufferedContent*/tempBuffer : outputBuffer;
    if (!inputBuffer.popToBuffer(targetBuffer, payloadSize))
    {
        return DecodeResult::Failure;
    }

    if (isMasked)
    {
        for (unsigned __int64 i = 0; i < payloadSize; i++) {
            targetBuffer.getBuffer()[i] = (targetBuffer.getBuffer()[i] ^ masks[i % 4]);
        }
    }

    if (shouldDoBuffering)
    {
        websocketContext.bufferedContent.addBuffer(targetBuffer);
    }

	/*unsigned int currentOffset = targetBuffer.getDataSize();
	if (!targetBuffer.append(inputBuffer.getBuffer(payloadOffset), payloadSize))
		return DecodeResult::Failure;
	if (isMasked)
	{
		for (unsigned __int64 i = 0; i < payloadSize; i++) {
			targetBuffer.getBuffer()[i + currentOffset] = (targetBuffer.getBuffer()[i + currentOffset] ^ masks[i%4]);
		}
	}*/

	bool isTextFrame = opCode == OpCode::Text ||
		(opCode == OpCode::Continuation && websocketContext.currentContinuationOpCode == OpCode::Text);

	if (isTextFrame && validateTextAsUtf8 && isFIN)
	{
		/*if (!validateUtf8Context(targetBuffer))
			return DecodeResult::Failure;*/
	}

	if (shouldDoBuffering)
	{
		if (isFIN)
		{
            if (!websocketContext.bufferedContent.popToBuffer(outputBuffer, websocketContext.bufferedContent.getDataSize()))
            {
                return DecodeResult::Failure;
            }

			websocketContext.currentContinuationOpCode = -1;
		}
		else
		{
			if(opCode!=OpCode::Continuation)
				websocketContext.currentContinuationOpCode = opCode;
			return DecodeResult::NoContent;
		}		
	}

	return DecodeResult::Content;
}

bool WebsocketProtocol::processHandshake( Buffer& request, Buffer& response )
{
	WebsocketHandshakeMessage hanshakeRq;
	if (!hanshakeRq.ParseFromRequest(request))
		return false;

	WebsocketHandshakeMessage handshakeResponse;

	std::string server_key = hanshakeRq.GetField("Sec-WebSocket-Key");
	server_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	CSHA1        sha;
	unsigned int    message_digest[5];

	sha.Reset();
	sha << server_key.c_str();

	sha.Result(message_digest);
	// convert sha1 hash bytes to network byte order because this sha1
	//  library works on ints rather than bytes
	for (int i = 0; i < 5; i++) {
		message_digest[i] = htonl(message_digest[i]);
	}

	server_key = base64_encode(reinterpret_cast<const unsigned char*>(message_digest), 20);


	handshakeResponse.SetField("Upgrade", "websocket");
	handshakeResponse.SetField("Connection", "Upgrade");
	handshakeResponse.SetField("Sec-WebSocket-Accept", server_key);


	return handshakeResponse.Serialize(response);;
}


EncodeResult::Type WebsocketProtocol::encodeContent( ProtocolContext& context, Buffer& inputBuffer, Buffer& outputBuffer )
{
	return encodeContent(context, inputBuffer, outputBuffer, isBinaryMode ? OpCode::Binary : OpCode::Text);
}

EncodeResult::Type WebsocketProtocol::encodeContent( ProtocolContext& context, Buffer& inputBuffer, Buffer& outputBuffer, int opCode )
{
	unsigned __int64 payloadSize = inputBuffer.getDataSize();

	int totalSize = payloadSize;
	totalSize += 1; //flags byte.

	// Add the size bytes:
	if(payloadSize <= 125  && payloadSize <= 65535 )
		totalSize += 1;
	else if(payloadSize > 125  && payloadSize <= 65535)
		totalSize += 3;
	else
		totalSize += 9;

	// Take into account the masks bytes if we are client:
	if (isClientSide)
		totalSize += 4;

    outputBuffer.allocate(totalSize);
/*
	if (totalSize > outputBuffer.getRemainingSize())
		return EncodeResult::InsufficientBufferSpace;*/

	//create the flags byte
	int index = 0;
	char payloadFlags = OpCode::getPayloadFlag(true, (OpCode::Type)opCode);

	outputBuffer.append(payloadFlags);

	//create the length byte
	if (payloadSize <= 125)
	{
		char basicSize = payloadSize;
		if(isClientSide)
			basicSize |= 0x80;
		outputBuffer.append(basicSize);
	}
	else if (payloadSize > 125 && payloadSize <= 65535)
	{
		char basicSize = 126;
		if(isClientSide)
			basicSize |= 0x80;

		outputBuffer.append(basicSize);

		char len[2];
		len[0] = ( payloadSize >> 8 ) & 255;
		len[1] = ( payloadSize ) & 255;

		outputBuffer.append(len, 2);
	}
	else
	{
		char basicSize = 127;
		if(isClientSide)
			basicSize |= 0x80;
		outputBuffer.append(basicSize);

		char len[8];
		len[0] = ( payloadSize >> 56 ) & 255;
		len[1] = ( payloadSize >> 48  ) & 255;
		len[2] = ( payloadSize >> 40 ) & 255;
		len[3] = ( payloadSize >> 32  ) & 255;
		len[4] = ( payloadSize >> 24 ) & 255;
		len[5] = ( payloadSize >> 16  ) & 255;
		len[6] = ( payloadSize >> 8 ) & 255;
		len[7] = ( payloadSize ) & 255;

		outputBuffer.append(len, 8);
	}

	char masks[4];
	if (isClientSide)
	{
		masks[0] = 180;
		masks[1] = 180;
		masks[2] = 180;
		masks[3] = 180;

		outputBuffer.append(masks, 4);
	}


	char* payloadPtr = outputBuffer.getPosition();
	outputBuffer.append(inputBuffer);

	if (isClientSide)
	{
		for (unsigned __int64 i = 0; i < payloadSize; i++) {
			payloadPtr[i+index] = (payloadPtr[i+index] ^ masks[i%4]);
		}
	}


	return EncodeResult::Success;
}

DecodeResult::Type WebsocketProtocol::tryDecodeHanshakeData( ProtocolContext& context, Buffer& outputBuffer )
{
	BufferList& inputBuffer = context.getDataBuffer();
	char httpEnd[4] = {'\r', '\n', '\r', '\n'};

	for (int i=0;i<inputBuffer.getDataSize();i++)
	{
		unsigned int remainingBytes = inputBuffer.getDataSize() - i;
		if (remainingBytes < 4)
			return DecodeResult::WantMoreData;

        char temp[4];
        inputBuffer.copyTo(i, temp, 4);

		if (strncmp(httpEnd, temp, 4) != 0)
			continue;

		if (!isClientSide)
		{            
            Buffer request(&getMsgBufferPool());
            inputBuffer.popToBuffer(request, i + 4);

            request.getBuffer()[i] =  '\0';
			processHandshake(request, outputBuffer);///TODO Handle failure and a way to send protocol failure response.

			context.setInitialized();
			return DecodeResult::ProtocolBytes;
		}
		else
		{
			inputBuffer.pop(i+4);
			context.setInitialized();
			return DecodeResult::NoContent;
		}
	}

	return DecodeResult::WantMoreData;
}

unsigned int WebsocketProtocol::getRequiredRecyclableBuffers( unsigned int nMaxConnections, unsigned int nMaxConcurrentCalls )
{
	unsigned int nTotal = 0;
	nTotal += (nMaxConnections * 1);
	nTotal += (nMaxConcurrentCalls * 1);
	return nTotal;
}

bool WebsocketProtocol::validateUtf8Context( Buffer& content )
{
	string str(content.getBuffer(), content.getDataSize());
	
	string::iterator end_it = utf8::find_invalid(str.begin(), str.end());

	return end_it == str.end();
}

void WebsocketProtocol::ValidateTextAsUtf8( bool val )
{
	validateTextAsUtf8 = val;
}

void WebsocketProtocol::validateCloseFramePayload( Buffer& payload )
{
	if(payload.isEmpty())
		return;

	if (payload.getDataSize() < 2)
		return;

	u_short uStatus = ntohs( *(u_short*)payload.getBuffer(0) );

	bool isValidStatus = false;

	if (uStatus == 1000 ||
		uStatus == 1001 ||
		uStatus == 1002 ||
		uStatus == 1003 ||
		uStatus == 1007 ||
		uStatus == 1008 ||
		uStatus == 1009 ||
		uStatus == 1010 ||
		uStatus == 1011)
	{
		isValidStatus = true;
	}


	if (!isValidStatus)
	{
		payload.clearBytes();
		return;
	}

	if(payload.getDataSize() == 2)
		return;

	/*Buffer closeTextPayload(&getMsgBufferPool());
	closeTextPayload.assign(payload.getBuffer(2), payload.getDataSize() - 2);

	if (validateUtf8Context(closeTextPayload))
	{
		return;
	}*/

	payload.clearBytes();
	return;
}
