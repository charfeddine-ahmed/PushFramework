/********************************************************************
	File :			stdafx.h
	Creation date :	2010/6/27

	License :			Copyright 2010 Ahmed Charfeddine, http://www.ProtocolFramework.com

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




#include <stdlib.h> //For itoa and C routines
#include <stdio.h>


#include <map>
#include <vector>
#include <set>

#include <string.h>
#include <iostream>
#include <fstream>

#include <sstream>
#include <math.h>

#include <time.h>
#include <unordered_set>
#include <unordered_map>

using namespace std;


#ifdef Plateform_Linux

#include "linuxcs.h"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netinet/in.h>
#include <sys/epoll.h>

#include <sys/time.h>

#include <pthread.h>

#include <termios.h>
#include <unistd.h>

#include <stdarg.h>

#ifndef SOCKET
typedef int SOCKET;
#endif

#endif

#include "ProtocolFrameworkInc.h"
using namespace ProtocolFramework;





