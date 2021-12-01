#include "StdAfx.h"
#include "LogicalConnectionPool.h"
#include "ServerImpl.h"
#include "LogicalConnectionImpl.h"
#include "../include/Server.h"
#include "../include/LogicalConnection.h"

namespace PushFramework
{

	LogicalConnectionPool::LogicalConnectionPool(ServerImpl* pServerImpl)
	{
		this->pServerImpl = pServerImpl;
	}


	LogicalConnectionPool::~LogicalConnectionPool(void)
	{
	}

	void LogicalConnectionPool::deleteImpl( ObjectPtr obj )
	{
		LogicalConnectionImpl* logicalConnection = (LogicalConnectionImpl*) obj;
		delete logicalConnection->getFacade();
	}

	void LogicalConnectionPool::recycleObject( ObjectPtr obj )
	{
		LogicalConnectionImpl* logicalConnection = (LogicalConnectionImpl*) obj;
		logicalConnection->Recycle();
	}

	AbstractPool::ObjectPtr LogicalConnectionPool::createImpl()
	{
		LogicalConnection* logicalConnection = pServerImpl->getFacade()->createLogicalConnection();
		return &logicalConnection->getImpl();
	}

}
