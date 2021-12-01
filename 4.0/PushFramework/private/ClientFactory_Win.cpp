/********************************************************************
	File :			ChannelFactory_Win.cpp
	Creation date :	2012/01/29

	License :			Copyright 2010 Ahmed Charfeddine, http://www.pushframework.com

				   Licensed under the Apache License, Version 2.0 (the "License");
				   you may not use this file except in compliance with the License.
				   You may obtain a copy of the License at

					   http://www.apache.org/licenses/LICENSE-2.0

				   Unless required by applicable law or agreed to in writing, software
				   distributed under the License is distributed on an "AS IS" BASIS,
				   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
				   See the License for the specific language governing permissions and
				   limitations under the License.


*********************************************************************/
#include "StdAfx.h"
#include "ClientFactory.h"
#include "ServerImpl.h"
#include "Dispatcher.h"
#include "../include/ServerOptions.h"
#include "PhysicalConnectionPool.h"
#include "PhysicalConnection.h"

#ifdef Plateform_Windows
#include "IOQueue_Win.h"
#elif Plateform_Linux
#include "IOQueue_Linux.h"
#endif

namespace PushFramework
{

bool ClientFactory::createPhysicalConnection( SOCKET hSocket, SOCKADDR_IN address, ListenerOptions* pListenerOptions )
{
	PhysicalConnection* connection = (PhysicalConnection*) pServerImpl->getPhysicalConnectionPool().borrowObject();

	// In both cases reset the object.
	connection->reset(hSocket, address, pListenerOptions);

	//
	if (!connection->SetUpProtocolContexts())
	{
		pServerImpl->getPhysicalConnectionPool().returnObject(connection);
		return false;
	}

    //Now Associate with IOCP main Handle
    if (!pServerImpl->getIOQueue().addSocketContext(hSocket, (DWORD) connection))
    {
		pServerImpl->getPhysicalConnectionPool().returnObject(connection);
        //leave socket close to Listener
        return false;
    }

	addPhysicalConnection(connection);

    //Unlocking iocp event:
    if(!pServerImpl->getIOQueue().initializeSocketContext( (DWORD) connection))
    {
        removePhysicalConnection(connection);
        pServerImpl->getPhysicalConnectionPool().returnObject(connection);
        //leave socket close to Listener
        return false;
    }

    return true;
}


}