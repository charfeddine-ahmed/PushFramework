#include "stdafx.h"
#include "ProtocolManager.h"

#include "Connection.h"
#include "DeserializeData.h"
#include "SerializeData.h"
#include "MessageFactory.h"
#include "Protocol.h"
#include "ProtocolContext.h"

namespace ProtocolFramework
{

    ProtocolManager::ProtocolManager(MemoryAllocator* pAllocator)
	{
        this->pAllocator = pAllocator;
	}

	ProtocolManager::~ProtocolManager(void)
	{
	}


    NetworkSerializeResult::Type ProtocolManager::serializeOutgoingBytes(Connection& connection, Buffer& outgoingBytes, Buffer& outputBuffer, SerializeData& serializeData)
	{		
        Buffer encodeInput(pAllocator);

		Protocol* pProtocol = serializeData.pProtocol;

        if (!pProtocol)
        {
            outputBuffer.assign(outgoingBytes, true);
            return NetworkSerializeResult::Success;
        }
        
        encodeInput.assign(outgoingBytes, false);

		while (pProtocol)
		{
            bool isLowestProtocol = pProtocol->getLowerProtocol() == NULL;

            Buffer tempBuffer(pAllocator);
            Buffer& output = isLowestProtocol ? outputBuffer : tempBuffer;
            EncodeResult::Type encodeResult = pProtocol->encodeContent(connection.getProtocolContext(pProtocol), encodeInput, output);
			
			if (encodeResult != EncodeResult::Success)
			{
				if (encodeResult == EncodeResult::InsufficientBufferSpace && (pProtocol->getLowerProtocol() != NULL))
					return NetworkSerializeResult::Failure; //Buffer overflow at an intermediate protocol layer is not allowable.

				return NetworkSerializeResult::convertEncodingFailure(encodeResult);
			}

            if (!isLowestProtocol)
            {
                //TODO.
                encodeInput.assign(tempBuffer, true);
            }

			pProtocol = pProtocol->getLowerProtocol();
		}

		return NetworkSerializeResult::Success;
	}

	NetworkSerializeResult::Type ProtocolManager::serializeOutgoingPacket( Connection& connection, OutgoingPacket& outgoingMsg, Buffer& outputBuffer, SerializeData& serializeData )
	{
		MessageFactory& msgFactory = connection.getMessageFactory();

		// First encode the message:
		Buffer packetBuffer(pAllocator);
		SerializeResult::Type ret = msgFactory.serializeMessage(outgoingMsg, packetBuffer);
		if (ret != SerializeResult::Success)
			return NetworkSerializeResult::convertSerializeFailure(ret);

		return serializeOutgoingBytes(connection, packetBuffer, outputBuffer, serializeData);
	}

	NetworkDeserializeResult::Type ProtocolManager::tryDeserializeIncomingPacket( Connection& connection, DeserializeData& deserializeData )
	{
		MessageFactory& msgFactory = connection.getMessageFactory();	
		
		while (true)
		{
			Protocol* pCurrentProtocol = deserializeData.pProtocol;
			ProtocolContext& currentContext = connection.getProtocolContext(pCurrentProtocol);
			Protocol* pUpperProtocol = pCurrentProtocol->getUpperProtocol();

            Buffer outputBuffer(pAllocator);

			//Give chance to protocol to call startSession before processing incoming data.
			if (!currentContext.isInitialized() && !currentContext.isInitializationStarted())
			{
				currentContext.setInitializationStarted();
				pCurrentProtocol->startSession(currentContext, outputBuffer);
				//
				if (outputBuffer.hasBytes())
				{
                    deserializeData.protocolBytes.assign(outputBuffer, true);
					return NetworkDeserializeResult::ProtocolBytes;
				}
			}
			
			DecodeResult::Type decodeResult = pCurrentProtocol->tryDecode(currentContext, outputBuffer);

			if (decodeResult == DecodeResult::WantMoreData)
			{
				if (currentContext.isInitialized() &&  pUpperProtocol)
				{
					deserializeData.pProtocol = pUpperProtocol;
					continue;
				}
				else
				{
					return NetworkDeserializeResult::WantMoreData;
				}				
			}
			else if (decodeResult == DecodeResult::NoContent)
			{
				continue;
			}
			else if (decodeResult == DecodeResult::ProtocolBytes)
			{
                deserializeData.protocolBytes.assign(outputBuffer, true);
				//
				return NetworkDeserializeResult::ProtocolBytes;
			}
			else if(decodeResult == DecodeResult::Close)
			{
				if (pUpperProtocol)
				{
                    deserializeData.protocolBytes.assign(outputBuffer, true);
				}

				return NetworkDeserializeResult::Close;
			}
			else if (decodeResult == DecodeResult::Content)
			{
				if (!currentContext.isInitialized())
				{
					return NetworkDeserializeResult::Initializationfailure;
				}

				if (!pUpperProtocol)
				{
					//De-serialize and return message.
					DeserializeResult::Type deserializeResult = msgFactory.deserializeMessage(outputBuffer, deserializeData.pMessage, deserializeData.nRoutingService);
					if (deserializeResult == DeserializeResult::DiscardContent)
					{
						continue;
					}
					else if (deserializeResult == DeserializeResult::Failure)
					{
						return NetworkDeserializeResult::Failure;
					}
					else if(deserializeResult == DeserializeResult::Success)
					{
						return NetworkDeserializeResult::Content;
					}
					else
					{
						//Should not come here.
					}
				}

				//Give content data to upper protocol:
				if (!pUpperProtocol->readData(connection.getProtocolContext(pUpperProtocol), outputBuffer))
				{
					return NetworkDeserializeResult::Failure;
				}
			}
			else //decodeResult == DecodeResult::Failure
			{
				return NetworkDeserializeResult::Failure; //
			}
		}
	}

	bool ProtocolManager::processIncomingData( Connection& connection, Buffer& incomingBytes )
	{
		Protocol* pLowestProtocol = connection.getProtocol()->getLowestProtocol();

		return pLowestProtocol->readData(connection.getProtocolContext(pLowestProtocol), incomingBytes);
	}

}
