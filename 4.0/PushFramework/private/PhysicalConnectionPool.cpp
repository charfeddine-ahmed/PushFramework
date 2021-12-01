#include "StdAfx.h"
#include "PhysicalConnectionPool.h"
#include "PhysicalConnection.h"

namespace PushFramework
{
	PhysicalConnectionPool::PhysicalConnectionPool(ServerImpl* pServerImpl)
	{
		this->pServerImpl = pServerImpl;
	}

	PhysicalConnectionPool::~PhysicalConnectionPool(void)
	{
	}

	void PhysicalConnectionPool::deleteImpl( ObjectPtr obj )
	{
		PhysicalConnection* connection = (PhysicalConnection*) obj;
		delete connection;
	}

	void PhysicalConnectionPool::recycleObject( ObjectPtr p )
	{
		PhysicalConnection* connection = (PhysicalConnection*) p;
		connection->Recycle();
	}

	AbstractPool::ObjectPtr PhysicalConnectionPool::createImpl()
	{
		return new PhysicalConnection(pServerImpl);
	}

}