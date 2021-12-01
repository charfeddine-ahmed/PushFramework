#include "stdafx.h"
#include "WebsocketContext.h"


WebsocketContext::WebsocketContext()
{
	currentContinuationOpCode = -1;
}


WebsocketContext::~WebsocketContext(void)
{
}

void WebsocketContext::recycle()
{
	bufferedContent.release();
	currentContinuationOpCode = -1;
}
