#include "globals.h"
#include "ButtonFieldState_v8_wrapper.h"
#include <v8.h>
#include "ButtonFieldState.h"

using namespace v8;


#ifdef UNICODE
# define V8_TCHAR uint16_t
#else
# define V8_TCHAR char
#endif


void AddButtonFieldStateAcessors(Handle<ObjectTemplate> &templ)
{
	HandleScope scope;
	
}
