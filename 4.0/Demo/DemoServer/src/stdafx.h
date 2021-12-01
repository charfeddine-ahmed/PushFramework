// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#include "Plateform.h"

#ifdef Plateform_Windows
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#endif


#include <stdio.h>

#include <ctype.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <string.h>

#include <string>
#include <map>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
using namespace std;

#include <ProtocolFrameworkInc.h>
using namespace ProtocolFramework;

#include <PushFrameworkInc.h>
using namespace PushFramework;

#include <json.h>
#include <JsonSerializer.h>

#include <SimpleProtocol.h>


#include "Configuration.h"
#include "..\common\typeids.h"

#ifdef Plateform_Linux
#define SleepMs(nMilliSeconds) usleep(nMilliSeconds * 1000)
#endif
#ifdef Plateform_Windows
#define SleepMs(nMilliSeconds) Sleep(nMilliSeconds)
#endif



