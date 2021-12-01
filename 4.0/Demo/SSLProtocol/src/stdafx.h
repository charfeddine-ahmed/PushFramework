// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>


#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#include <ctype.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <string.h>

#include <map>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
using namespace std;

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <ProtocolFrameworkInc.h>
using namespace ProtocolFramework;