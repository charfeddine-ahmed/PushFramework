#include "stdafx.h"
#include "Protocol.h"
#include "Buffer.h"
#include "ErrorCodes.h"
#include "ProtocolContext.h"

namespace ProtocolFramework
{
	
	Protocol::Protocol( void )
	{
		pUpperProtocol = NULL;
		pLowerProtocol = NULL;
	}

	Protocol::~Protocol( void )
	{
		//
	}

	Protocol* Protocol::getLowerProtocol()
	{
		return pLowerProtocol;
	}

	Protocol* Protocol::getUpperProtocol()
	{
		return pUpperProtocol;
	}

	Protocol* Protocol::getLowestProtocol()
	{
		if (pLowerProtocol)
		{
			return pLowerProtocol->getLowestProtocol();
		}

		return this;
	}

	void Protocol::addLowerProtocolLayer( Protocol* pProtocol )
	{
		pLowerProtocol = pProtocol;
		pProtocol->pUpperProtocol = this;
	}

	void Protocol::deleteImpl( ObjectPtr obj )
	{
		ProtocolContext* context = (ProtocolContext*) obj;
		delete context;
	}

	AbstractPool::ObjectPtr Protocol::createImpl()
	{
		return createNewProtocolContext();
	}

	void Protocol::recycleObject( ObjectPtr p )
	{
		ProtocolContext* context = (ProtocolContext*) p;
		context->recycleIntern();
	}

	const char* Protocol::getName()
	{
		return "undefined";
	}

    MemoryAllocator& Protocol::getMsgBufferPool()
	{
		return *pAllocator;
	}

    bool Protocol::initialize(unsigned int nMaxContexts, MemoryAllocator* pAllocator)
	{
        this->pAllocator = pAllocator;

		if (!AbstractPool::initialize(nMaxContexts))
		{
			return false;
		}
		
		return true;
	}

}

