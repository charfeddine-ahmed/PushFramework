
#include "StdAfx.h"
#include "ClientFactory.h"
#include "ServerImpl.h"
#include "Dispatcher.h"
#include "../include/ServerOptions.h"
#include "PhysicalConnection.h"
#include "PhysicalConnectionPool.h"


#ifdef Plateform_Windows
#include "IOQueue_Win.h"
#else
#ifdef Plateform_Linux
#include "IOQueue_Linux.h"
#endif
#endif

namespace PushFramework
{

bool ClientFactory::createPhysicalConnection( SOCKET hSocket, SOCKADDR_IN address, ListenerOptions* pListenerOptions )
{  
	PhysicalConnection* connection = (PhysicalConnection*)pServerImpl->getPhysicalConnectionPool().borrowObject();
    // In both cases reset the object.
    connection->reset(hSocket, address, pListenerOptions);
    //
	if (!connection->SetUpProtocolContexts())
	{
		pServerImpl->getPhysicalConnectionPool().returnObject(connection);
		return false;
	}
        
    addPhysicalConnection(connection);

    //Now Associate with IOQueue
	if (!pServerImpl->getIOQueue().addSocketContext(hSocket, connection))
    {
        removePhysicalConnection(connection);
		pServerImpl->getPhysicalConnectionPool().returnObject(connection);
        //leave socket close to acceptor
        return false;
    }
 
    pServerImpl->getDispatcher().handleInitialize(*connection);
    return true;
}
}
