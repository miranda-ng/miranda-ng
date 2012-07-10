#include "globals.h"
#include "IconFieldState_v8_wrapper.h"
#include <v8.h>
#include "IconFieldState.h"

using namespace v8;


#ifdef UNICODE
# define V8_TCHAR uint16_t
#else
# define V8_TCHAR char
#endif


void AddIconFieldStateAcessors(Handle<ObjectTemplate> &templ)
{
	HandleScope scope;
	
}
