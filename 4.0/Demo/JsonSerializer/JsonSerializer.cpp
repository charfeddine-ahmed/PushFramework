// JsonSerializer.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "JsonSerializer.h"

SerializeResult::Type JsonSerializer::serializeMessage(OutgoingPacket& outgoingPacket, Buffer& buffer)
{
	Json::Value& jsonVal = reinterpret_cast<Json::Value&> (outgoingPacket);

	Json::StyledWriter writer;
	string dataMessage = writer.write(jsonVal);

    if (!buffer.allocate(dataMessage.length()))
    {
        return SerializeResult::Failure;
    }

	buffer.append(dataMessage.c_str(), dataMessage.length());

	return SerializeResult::Success;
}

DeserializeResult::Type JsonSerializer::deserializeMessage( Buffer& contentBytes, IncomingPacket*& pMessage, int& nRoutingService )
{
	Json::Value* jsonVal = new Json::Value();
	Json::Reader reader;

	const char *beginDoc = contentBytes.getBuffer();
	const char *endDoc = contentBytes.getPosition();

	if (!reader.parse(beginDoc, endDoc, *jsonVal, false)){
		cout << "failed to deserialized json" << endl;
		return DeserializeResult::Failure;
	}

	nRoutingService = jsonVal->get("routingId", 0).asInt();
	
	pMessage = reinterpret_cast<IncomingPacket*> (jsonVal);
	return DeserializeResult::Success;
}

void JsonSerializer::disposeIncomingPacket( IncomingPacket* pPacket )
{
	Json::Value* jsonVal = reinterpret_cast<Json::Value*> (pPacket);
	delete jsonVal;
}

void JsonSerializer::disposeOutgoingPacket( OutgoingPacket* pPacket )
{
	Json::Value* jsonVal = reinterpret_cast<Json::Value*> (pPacket);
	delete jsonVal;
}


