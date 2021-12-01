// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the TCPSOCKET_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// TCPSOCKET_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#include "Plateform.h"

//Export/Import symbols :
#ifdef Plateform_Windows

#ifdef TCPSOCKET_EXPORTS
#define TCPSOCKET_API __declspec(dllexport)
#else
#define TCPSOCKET_API __declspec(dllimport)
#endif

#else

#ifdef TCPSOCKET_EXPORTS
#define TCPSOCKET_API
#else
#define TCPSOCKET_API
#endif

#endif

