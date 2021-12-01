#ifndef ProtocolContext__INCLUDED
#define ProtocolContext__INCLUDED

#pragma once
#include "ProtocolFramework.h"
#include "BufferList.h"

namespace ProtocolFramework
{
	class PROTOCOLFRAMEWORK_API ProtocolContext
	{
	public:
		ProtocolContext();
		virtual ~ProtocolContext(void);
		
        BufferList& getDataBuffer();
		bool isInitialized();
		bool isInitializationStarted();
		void setInitializationStarted();
		void setInitialized();
		
		void recycleIntern();
	private:
        BufferList receivedData;
		short state;

	protected:
		virtual void recycle();
	};

}
#endif // ProtocolContext__INCLUDED