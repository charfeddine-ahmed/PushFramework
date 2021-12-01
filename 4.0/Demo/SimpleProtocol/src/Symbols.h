
#include "Plateform.h"


//Export/Import symbols :
#ifdef Plateform_Windows
#ifdef SIMPLEPROTOCOL_EXPORTS
#define SIMPLEPROTOCOL_API __declspec(dllexport)
#else
#define SIMPLEPROTOCOL_API __declspec(dllimport)
#endif

#else

#ifdef Plateform_Linux
#define SIMPLEPROTOCOL_API
#endif

#endif