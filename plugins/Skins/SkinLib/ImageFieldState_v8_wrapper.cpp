#include "globals.h"
#include "ImageFieldState_v8_wrapper.h"
#include <v8.h>
#include "ImageFieldState.h"

using namespace v8;


#ifdef UNICODE
# define V8_TCHAR uint16_t
#else
# define V8_TCHAR char
#endif


void AddImageFieldStateAcessors(Handle<ObjectTemplate> &templ)
{
	HandleScope scope;
	
}
