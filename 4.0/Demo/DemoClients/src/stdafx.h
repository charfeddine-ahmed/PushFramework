// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__TOS_LINUX__)
#define Plateform_Linux
#elif defined(_WIN32) || defined(_WIN64)
#define Plateform_Windows
#endif


#include <stdio.h>
#include <tchar.h>

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

#include <json.h>
#include <JsonSerializer.h>
#include <SimpleProtocol.h>

#include <TcpSocket.h>




#include "Configuration.h"

#include "..\common\typeids.h"
